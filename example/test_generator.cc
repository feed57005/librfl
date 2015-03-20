#include "rfl/reflected.h"
#include "rfl/generator.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>
#include <algorithm>

namespace rfl {
namespace {

static std::string GenClassVarName(Class const *klass) {
  std::string var_name = "__c_" + (klass->parent_class() != nullptr
                                       ? (klass->parent_class()->name() + "_")
                                       : "") +
                         klass->name();
  return var_name;
}

static std::string EscapeAnnotation(std::string const &anno) {
  std::string ret = anno;

  size_t i = 0;
  while (i < ret.length()) {
    if (ret[i] == '"') {
      ret.insert(i, 1, '\\');
      i++;
    }
    i++;
  }
  return ret;
}
} // namespace

class BasicGenerator : public Generator {
public:
  virtual ~BasicGenerator() {}

protected:
  virtual int BeginPackage(Package const *pkg);
  virtual int EndPackage(Package const *pkg);
  virtual int BeginClass(Class const *klass);
  virtual int EndClass(Class const *klass);
  virtual int BeginNamespace(Namespace const *ns);
  virtual int EndNamespace(Namespace const *ns);

protected:
  std::stringstream out_;

  std::deque<std::string> namespaces_stack_;
  std::deque<std::string> classes_stack_;
  std::vector<std::string> includes_;
};

int BasicGenerator::BeginPackage(Package const *pkg) {
  out_ << "#include \"rfl/reflected.h\"\n";
  out_ << "using namespace rfl;\n";

  namespaces_stack_.push_front("static Namespace *__NS__[] = {\n");

  return 0;
}

int BasicGenerator::EndPackage(Package const *pkg) {

  std::string &namespaces = namespaces_stack_.front();
  namespaces += "  nullptr\n};\n";
  out_ << namespaces << "\n";
  namespaces_stack_.pop_front();

  out_ << "static Package __package__ = Package(\"" << pkg->name() << "\", \""
       << pkg->version() << "\", __NS__);\n\n";

  out_ << "extern \"C\" {\n";
  out_ << "rfl::Package const *LoadPackage() {\n";
  out_ << "  return &__package__;\n";
  out_ << "}\n";
  out_ << "} // extern \"C\"\n";
  out_ << "\n";

  std::ofstream file_out;
  std::string path = output_path_;
  path += ".cc";
  file_out.open(path, std::ios_base::out);
  for (std::string const &include : includes_) {
    file_out << "#include \"" << include << "\"\n";
  }
  file_out << "\n";
  file_out << out_.str();
  file_out.close();
  return 0;
}

int BasicGenerator::BeginNamespace(Namespace const *ns) {
  out_ << "\n";
  out_ << "using namespace " << ns->name() << ";\n\n";

  std::string namespaces = "static Namespace *__NS_" + ns->name() + " = {\n";
  std::string classes = "static Class *__C_" + ns->name() + "[] = {\n";

  namespaces_stack_.push_front(namespaces);
  classes_stack_.push_front(classes);
  return 0;
}

int BasicGenerator::EndNamespace(Namespace const *ns) {
  std::string &namespaces = namespaces_stack_.front();
  namespaces += "  nullptr\n};\n";

  std::string &classes = classes_stack_.front();
  classes += "  nullptr\n};\n";

  if (ns->GetNumNamespaces() > 0) {
    out_ << namespaces << "\n";
  }
  out_ << classes << "\n";
  out_ << "static Namespace __ns_" << ns->name() << " =\n";
  out_ << "    Namespace(\"" << ns->name() << "\",\n";
  out_ << "              __C_" << ns->name() << ",\n";
  if (ns->GetNumNamespaces() > 0) {
    out_ << "              _NS_" << ns->name() << ");\n";
  } else {
    out_ << "              nullptr);\n";
  }

  namespaces_stack_.pop_front();
  classes_stack_.pop_front();

  namespaces_stack_.front() += "  &__ns_" + ns->name() + ",\n";
  return 0;
}

int BasicGenerator::BeginClass(Class const *klass) {
  std::string var_name = GenClassVarName(klass);
  // nested classes
  std::string nested_classes = "static Class *__C_" + var_name + "[] = {\n";
  classes_stack_.push_front(nested_classes);
  return 0;
}

int BasicGenerator::EndClass(Class const *clazz) {
  std::string var_name = GenClassVarName(clazz);
  std::string class_name = clazz->name();
  class_name += "Class";

  // properties
  std::string props = "static Property *__P_" + var_name + "[] = {\n";
  for (size_t i = 0; i < clazz->GetNumProperties(); i++) {
    Property *prop = clazz->GetPropertyAt(i);
    out_ << "static Property __p_" << var_name << "_" << prop->name() << " =\n";
    out_ << "    Property(\"" << prop->name() << "\", \"" << prop->type() << "\","
        << "             " << prop->offset() << ",\n"
        << " Annotation(\"" << EscapeAnnotation(prop->annotation().value_)
        << "\", \"" << prop->annotation().file_ << "\", "
        << prop->annotation().line_ << "));\n";
    props += "  &__p_" + var_name + "_" + prop->name() + ",\n";
  }
  props += "  nullptr\n};\n";

  std::string &nested_classes = classes_stack_.front();
  nested_classes += "  nullptr\n};\n";

  out_ << props;
  if (clazz->GetNumClasses() > 0) {
    out_ << nested_classes;
  }

  out_ << "class " << class_name << " : public Class {\n";
  out_ << "public:\n";
  out_ << "  " << class_name << "()\n";
  out_ << "    : Class(\"" << clazz->name() << "\",\n";
  out_ << "            \"" << clazz->header_file() << "\",\n";
  out_ << "            Annotation(\""
      << EscapeAnnotation(clazz->annotation().value_) << "\", \""
      << clazz->annotation().file_ << "\", " << clazz->annotation().line_
      << "),\n";
  //  properties
  out_ << "            __P_" << var_name << ",\n";
  //  nested
  if (clazz->GetNumClasses() > 0) {
    out_ << "            __C_" << var_name << ",\n";
  } else {
    out_ << "            nullptr,\n";
  }
  if (clazz->super_class()) {
    out_ << "            &__c_" << clazz->super_class()->name() << ") {}\n";
  } else {
    out_ << "            nullptr) {}\n";
  }
  out_ << "  virtual void *CreateInstance() { return new " << clazz->name() << "(); }\n";
  out_ << "};\n";
  out_ << "static " << class_name << " " << var_name << " = " << class_name << "();\n";

  if (!clazz->header_file().empty()) {
    if (std::find(includes_.begin(), includes_.end(), clazz->header_file()) ==
        includes_.end())
      includes_.push_back(clazz->header_file());
  }

  classes_stack_.pop_front();
  classes_stack_.front() += "  &" + var_name + ",\n";
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

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

  std::vector<std::string> src_includes_;
  std::vector<std::string> h_includes_;
  std::vector<std::pair<std::string, std::string> > classes_;
};

void Gen::AddInclude(std::string const &inc, std::vector<std::string> &includes) {
  std::vector<std::string>::const_iterator it =
      std::find(includes.begin(), includes.end(), inc);
  if (it == includes.end())
    includes.push_back(inc);
}

int Gen::BeginPackage(Package const *pkg) {
  generated_package_ = pkg;
  // TODO generate header guard
  //hout_ << "#include \"\"\n\n";

  std::string header_file = output_path_;
  header_file += ".h";
  AddInclude(header_file, src_includes_);
  return 0;
}

int Gen::EndPackage(Package const *) {
  out_ << "extern \"C\" bool LoadPackage() {\n"
       << "  test::ClassRepository *repo = test::ClassRepository::GetSharedInstance();\n\n";
  for (std::pair<std::string, std::string> const &klass : classes_) {
    out_ << "  repo->RegisterClass(new " << klass.first << "());\n";
  }
  out_ << "  return true;\n"
       << "}\n";

  std::ofstream file_out;
  std::string path = output_path_;
  path += ".cc";
  file_out.open(path, std::ios_base::out);
  for (std::string const &include : src_includes_) {
    file_out << "#include \"" << include << "\"\n";
  }
  file_out << "\n";
  file_out << out_.str();
  file_out.close();

  path = output_path_;
  path += ".h";
  file_out.open(path, std::ios_base::out);
  for (std::string const &include : h_includes_) {
    file_out << "#include \"" << include << "\"\n";
  }
  file_out << "\n";
  file_out << hout_.str();
  file_out.close();

  generated_package_ = nullptr;
  return 0;
}

int Gen::BeginNamespace(Namespace const *ns) {
  out_ << "\n";
  out_ << "namespace " << ns->name() << " {\n\n";

  hout_ << "namespace " << ns->name() << " {\n\n";

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

  hout_ << "class " << class_name;
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
        << "  static ClassId ID;\n\n"
        << "  " << class_name << "();\n"
        << "  virtual ~" << class_name << "() {}\n\n"

        << "  virtual test::Object *CreateInstance();\n"
        << "  virtual void ReleaseInstance(test::Object *obj);\n\n"

        << "protected:\n"
        << "  explicit " << class_name << "(char const *name, ClassId parent_id);\n"
        << "  virtual bool InitClassProperties(ClassInstance *instance);\n"
        << "};\n\n";

  out_ << "ClassId " << class_name << "::ID = -1;\n\n";

  out_ << class_name << "::" << class_name << "() : "
          << parent_class_name << "(\"" << clazz->name() << "\", "
                               << parent_class_name << "::ID) {}\n\n";
  out_ << class_name << "::" << class_name
       << "(char const *name, ClassId parent_id) : " << parent_class_name
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
