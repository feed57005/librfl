#include "rfl/reflected.h"
#include "rfl/generator.h"

#include "package_manifest.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>
#include <algorithm>

namespace rfl {

class Gen : public Generator {
public:
  virtual ~Gen() {}

protected:
  virtual int BeginPackage(Package const *pkg);
  virtual int EndPackage(Package const *pkg);
  virtual int BeginClass(Class const *klass);
  virtual int EndClass(Class const *klass);
  virtual int BeginNamespace(Namespace const *ns);
  virtual int EndNamespace(Namespace const *ns);

  void AddInclude(std::string const &inc, std::vector<std::string> &includes);
protected:
  std::stringstream out_;
  std::stringstream hout_;
  Package const *generated_package_;
  std::string package_upper_;
  std::string header_prologue_;

  std::vector<std::string> src_includes_;
  std::vector<std::string> h_includes_;
  std::vector<std::pair<std::string, std::string> > classes_;
  std::vector<Enum *> enums_;
};

void Gen::AddInclude(std::string const &inc, std::vector<std::string> &includes) {
  std::vector<std::string>::const_iterator it =
      std::find(includes.begin(), includes.end(), inc);
  if (it == includes.end())
    includes.push_back(inc);
}

int Gen::BeginPackage(Package const *pkg) {
  generated_package_ = pkg;

  // header guard
  std::stringstream hguard;
  package_upper_ = pkg->name();
  std::transform(package_upper_.begin(), package_upper_.end(), package_upper_.begin(), ::toupper);
  hout_ << "#ifndef __" << package_upper_ << "_RFL_H__\n"
        << "#define __" << package_upper_ << "_RFL_H__\n\n";

  hout_ << "#if defined(WIN32)\n"
        << "#if defined(" << package_upper_ << "_IMPLEMENTATION)\n"
        << "#define " << package_upper_ << "_EXPORT __declspec(dllexport)\n"
        << "#else\n"
        << "#define " << package_upper_ << "_EXPORT __declspec(dllimport)\n"
        << "#endif\n\n"
        << "#else // defined(WIN32)\n"
        << "#if defined(" << package_upper_ << "_IMPLEMENTATION)\n"
        << "#define " << package_upper_ << "_EXPORT __attribute__((visibility(\"default\")))\n"
        << "#else\n"
        << "#define " << package_upper_ << "_EXPORT\n"
        << "#endif\n"
        << "#endif\n";

  header_prologue_ = hout_.str();

  hout_.str(std::string());


  std::string header_file = output_path_;
  header_file += "_rfl.h";
  AddInclude(header_file, src_includes_);
  return 0;
}

std::string GetLibraryForName(std::string const &name, char const *suffix) {
  std::string lib_name;
#if defined(__APPLE__) || defined(__linux__)
  lib_name = "lib";
#endif
  lib_name += name;
  if (suffix != nullptr)
    lib_name += suffix;
#if defined(__APPLE__)
  lib_name += ".dylib";
#elif defined(__linux__)
  lib_name += ".so";
#elif defined(WIN32)
  lib_name += ".dll";
#endif
  return lib_name;
}

int Gen::EndPackage(Package const *pkg) {
  hout_ << "#endif // __" << package_upper_ << "_RFL_H__\n";

  out_ << "extern \"C\" bool LoadPackage() {\n"
       << "  test::ClassRepository *repo = test::ClassRepository::GetSharedInstance();\n\n";
  for (Enum *e : enums_) {
    out_ << "  {\n"
         << "    test::Enum *" << e->name() << "_enum = new test::Enum();\n"
         << "    " << e->name() << "_enum->enum_name_ = \"" << e->name() << "\";\n";
    for (size_t j = 0; j < e->GetNumEnumItems(); ++j) {
      Enum::EnumItem const &item = e->GetEnumItemAt(j);
      out_ << "    " << e->name() << "_enum->items_.push_back(test::EnumItem(\""
           << item.id_ << "\",\"" << item.name_ << "\", " << item.value_
           << "));\n";
    }
    out_ << "    repo->RegisterEnum(" << e->name() << "_enum);\n";
    out_ << "  }\n";
  }
  for (std::pair<std::string, std::string> const &klass : classes_) {
    out_ << "  repo->RegisterClass(new " << klass.first << "());\n";
  }
  out_ << "  return true;\n"
       << "}\n";

  std::ofstream file_out;
  std::string path = output_path_;
  path += "_rfl.cc";
  file_out.open(path, std::ios_base::out);
  for (std::string const &include : src_includes_) {
    file_out << "#include \"" << include << "\"\n";
  }
  file_out << "\n";
  file_out << out_.str();
  file_out.close();

  path = output_path_;
  path += "_rfl.h";
  file_out.open(path, std::ios_base::out);
  file_out << header_prologue_;
  for (std::string const &include : h_includes_) {
    file_out << "#include \"" << include << "\"\n";
  }
  file_out << "\n";
  file_out << hout_.str();
  file_out.close();

  rfl::PackageManifest manifest;
  manifest.SetEntry("package.name", generated_package_->name().c_str());
  manifest.SetEntry("package.version", generated_package_->version().c_str());

  std::string lib_name = GetLibraryForName(output_path_, "_rfl");
  manifest.SetEntry("package.library", lib_name.c_str());

  for (int i = 0; i < generated_package_->GetImportNum(); i++) {
    std::string const &import = generated_package_->GetImportAt(i);
    std::string key = "imports.";
    key+= import;
    std::string val = GetLibraryForName(import, "_rfl");
    manifest.SetEntry(key.c_str(), val.c_str());
  }
#if 0
  for (int i = 0; i < generated_package_->GetLibraryNum(); i++) {
    std::string const &lib = generated_package_->GetLibraryAt(i);
    std::string key = "libs.";
    key+= lib;
    std::string val = lib;
    val += "_rfl";
    manifest.SetEntry(key.c_str(), "");
  }
#endif
  std::string manifest_file = output_path_;
  manifest_file += ".ini";
  manifest.Save(manifest_file.c_str());

  generated_package_ = nullptr;
  return 0;
}

int Gen::BeginNamespace(Namespace const *ns) {
  out_ << "\n";
  out_ << "namespace " << ns->name() << " {\n\n";

  hout_ << "namespace " << ns->name() << " {\n\n";

  for (size_t i = 0; i < ns->GetNumEnums(); ++i) {
    enums_.push_back(ns->GetEnumAt(i));
  }

  return 0;
}

int Gen::EndNamespace(Namespace const *ns) {
  hout_ << "} // namespace " << ns->name() << "\n\n";
  out_ << "} // namespace " << ns->name() << "\n\n";
  return 0;
}

int Gen::BeginClass(Class const *) {
  return 0;
}

static std::string GetFullClassName(Class const *clazz) {
  std::string ns="";
  Namespace const *current_ns = clazz->class_namespace();
  while (current_ns && current_ns->parent_namespace()) {
    ns.insert(0, "::");
    ns.insert(0, current_ns->name());
    current_ns = current_ns->parent_namespace();
  }
  ns+= clazz->name();
  ns+= "Class";
  return ns;
}

int Gen::EndClass(Class const *clazz) {
  std::string pkg = clazz->annotation().GetEntry("package");
  if (pkg.compare(generated_package_->name()) != 0) {
    std::string inc = pkg;
    inc+= "_rfl.h";
    AddInclude(inc, h_includes_);
    return 0;
  }

  AddInclude(clazz->annotation().file_, src_includes_);

  std::string parent_class_name;
  std::string class_name = clazz->name();
  class_name += "Class";
  classes_.push_back(std::make_pair(GetFullClassName(clazz), clazz->name()));


  hout_ << "// generated from: " << clazz->annotation().file_//header_file()
        << " " << pkg << "\n";

  hout_ << "class " << package_upper_ << "_EXPORT " << class_name;
  if (clazz->super_class() != nullptr) {
    parent_class_name = clazz->super_class()->name();
    parent_class_name += "Class";
    hout_ << " : public " << parent_class_name;
  } else {
    parent_class_name = "ObjectClass";
    hout_ << " : public test::ObjectClass";
    AddInclude("object.h", h_includes_);
  }
  hout_ << " {\n"
        << "public:\n"
        << "  static TypeId ID;\n\n"
        << "  " << class_name << "();\n"
        << "  virtual ~" << class_name << "() {}\n\n"

        << "  virtual test::Object *CreateInstance();\n"
        << "  virtual void ReleaseInstance(test::Object *obj);\n\n"

        << "protected:\n"
        << "  explicit " << class_name << "(char const *name, TypeId parent_id);\n"
        << "  virtual bool InitClassProperties(ClassInstance *instance);\n"
        << "};\n\n";

  out_ << "TypeId " << class_name << "::ID = -1;\n\n";

  out_ << class_name << "::" << class_name << "() : "
          << parent_class_name << "(\"" << clazz->name() << "\", "
                               << parent_class_name << "::ID) {}\n\n";
  out_ << class_name << "::" << class_name
       << "(char const *name, TypeId parent_id) : " << parent_class_name
       << "(name, parent_id) {}\n\n";
  out_ << "test::Object *" << class_name << "::CreateInstance() {\n"
       << "  return new " << clazz->name() << "();\n"
       << "}\n\n"

       << "void " << class_name << "::ReleaseInstance(test::Object *obj) {\n"
       << "  delete obj;\n"
       << "}\n\n";

  out_ << "bool " << class_name << "::InitClassProperties(ClassInstance *instance) {\n"
       << "  " << class_name << "::ID = instance->class_id_;\n";
  for (size_t i = 0; i < clazz->GetNumProperties(); i++) {
    Property *prop = clazz->GetPropertyAt(i);
    Annotation const &anno = prop->annotation();
    std::string kind = anno.GetEntry("kind");
    char const *id = anno.GetEntry("id");
    char const *name = anno.GetEntry("name");
    if (kind.compare("number") == 0) {
      char const *default_value = anno.GetEntry("default");
      char const *min = anno.GetEntry("min");
      char const *max = anno.GetEntry("max");
      char const *step = anno.GetEntry("step");
      char const *page_step = anno.GetEntry("page_step");
      char const *page_size = anno.GetEntry("page_size");
      char const *precision = anno.GetEntry("precision");
      out_ << "  instance->AddProperty(new test::NumericProperty<" << prop->type() << ">(\"" << id
           << "\", \"" << name << "\", " << prop->offset() << ", class_id(), "
           << "rfl::AnyVar((" << prop->type() << ")(" << default_value << ")), "
           << min << ", "
           << max << ", "
           << step << ", "
           << page_step << ", "
           << page_size << ", "
           << precision
           << "));\n";
    } else if (kind.compare("enum") == 0) {
      out_ << "  Enum *enum_" << id << "\n"
           << "   = test::ClassRepository::GetSharedInstance()->GetEnumByName(\""
           << prop->type() << "\");\n";
      out_ << "  instance->AddProperty(new test::EnumProperty(enum_" << id
           << ", \"" << id << "\", \"" << name << "\", " << prop->offset()
           << ", " << prop->type() << "::" << anno.GetEntry("default")
           << "));\n";
    } else {
      char const *default_value = anno.GetEntry("default");
      out_ << "  instance->AddProperty(new test::Property(\"" << id << "\", \""
           << name << "\", " << prop->offset() << ", class_id(), "
           << "rfl::AnyVar((" << prop->type() << ")(" << default_value << "))"
           << "));\n";
    }
  }
  out_ << "  return true;\n"
       << "}\n\n";

  return 0;
}

} // namespace rfl

using namespace rfl;

extern "C"
int GeneratePackage(char const *path, Package *pkg) {
  std::cout << "package: " << pkg->name() << "-" << pkg->version() << std::endl;
  Gen bg;
  bg.set_output_path(path);
  return bg.Generate(pkg);
}
