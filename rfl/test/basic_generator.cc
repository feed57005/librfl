#include "rfl/generator.h"
#include <sstream>
#include <deque>
#include <vector>

namespace rfl {

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

} // namespace rfl
