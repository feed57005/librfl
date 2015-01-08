#include "rfl/reflected_io.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace rfl {
#define indent(L) std::string((L)*2, ' ')

void DumpAnnotation(std::ostream &out, Annotation const &anno, int level) {
  out << indent(level) << "<annotation>" << anno.value_ << "</annotation>\n";
}

void DumpClass(std::ostream &out, Class const *klass, int level) {
  out << indent(level) << "<class name=\"" << klass->name() << "\"";
  if (klass->super_class())
    out << " super=\"" << klass->super_class()->name() << "\"";
  if (klass->parent_class())
    out << " parent=\"" << klass->parent_class()->name() << "\"";
  out << ">\n";
  DumpAnnotation(out, klass->annotation(), level+1);
  for (size_t i = 0; i < klass->GetNumClasses(); i++){
    DumpClass(out, klass->GetClassAt(i), level+1);
  }
  out << indent(level+1) << "<properties>\n";
  for (size_t i = 0; i < klass->GetNumProperties(); i++){
    Property *prop = klass->GetPropertyAt(i);
    out << indent(level + 2) << "<property name=\"" << prop->name()
        << "\" type=\"" << prop->type() << "\">\n";
    DumpAnnotation(out, prop->annotation(), level+3);
    out << indent(level+2) << "</property>\n";
  }
  out << indent(level+1) << "</properties>\n";
  out << indent(level) << "</class>\n";
}

void DumpNamespace(std::ostream &out, Namespace const *ns, int level) {
  out << indent(level) << "<namespace name=\"" << ns->name() << "\">\n";
  for (size_t i = 0; i < ns->GetNumNamespaces(); i++) {
    DumpNamespace(out, ns->GetNamespaceAt(i), level+1);
  }
  for (size_t i = 0; i < ns->GetNumClasses(); i++) {
    DumpClass(out, ns->GetClassAt(i), level+1);
  }
  out << indent(level) << "</namespace>\n";
}

void DumpPackage(std::ostream &out, Package const *pkg) {
  out << "<?xml version=\"1.0\"?>\n";
  out << "<package name=\"" << pkg->name() << "\" version=\"" << pkg->version() << "\">\n";
  for (size_t i = 0; i < pkg->GetNumNamespaces(); i++) {
    DumpNamespace(out, pkg->GetNamespaceAt(i), 1);
  }
  out << "</package>\n";
}
////////////////////////////////////////////////////////////////////////////////

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

static void GenerateClass(std::ostream &out,
                          Class const *clazz,
                          Class const *parent,
                          std::string &classes,
                          std::vector<std::string> &includes) {
  std::string var_name = "__c_" +
                         (parent != nullptr ? (parent->name() + "_") : "") +
                         clazz->name();
  std::string class_name = clazz->name();
  class_name += "Class";

  // nested classes
  std::string nested_classes = "static Class *__C_" + var_name + "[] = {\n";
  for (size_t i = 0; i < clazz->GetNumClasses(); i++) {
    GenerateClass(out, clazz->GetClassAt(i), clazz, nested_classes, includes);
  }

  // properties
  std::string props = "static Property *__P_" + var_name + "[] = {\n";
  for (size_t i = 0; i < clazz->GetNumProperties(); i++) {
    Property *prop = clazz->GetPropertyAt(i);
    out << "static Property __p_" << var_name << "_" << prop->name() << " =\n";
    out << "    Property(\"" << prop->name() << "\", \"" << prop->type() << "\","
        << "             " << prop->offset() << ",\n"
        << " Annotation(\"" << EscapeAnnotation(prop->annotation().value_)
        << "\", \"" << prop->annotation().file_ << "\", "
        << prop->annotation().line_ << "));\n";
    props += "  &__p_" + var_name + "_" + prop->name() + ",\n";
  }

  props += "  nullptr\n};\n";
  nested_classes += "  nullptr\n};\n";

  out << props;
  if (clazz->GetNumClasses() > 0) {
    out << nested_classes;
  }

  out << "class " << class_name << " : public Class {\n";
  out << "public:\n";
  out << "  " << class_name << "()\n";
  out << "    : Class(\"" << clazz->name() << "\",\n";
  out << "            \"" << clazz->header_file() << "\",\n";
  out << "            Annotation(\""
      << EscapeAnnotation(clazz->annotation().value_) << "\", \""
      << clazz->annotation().file_ << "\", " << clazz->annotation().line_
      << "),\n";
  //  properties
  out << "            __P_" << var_name << ",\n";
  //  nested
  if (clazz->GetNumClasses() > 0) {
    out << "            __C_" << var_name << ",\n";
  } else {
    out << "            nullptr,\n";
  }
  if (clazz->super_class()) {
    out << "            &__c_" << clazz->super_class()->name() << ") {}\n";
  } else {
    out << "            nullptr) {}\n";
  }
  out << "  virtual void *CreateInstance() { return new " << clazz->name() << "(); }\n";
  out << "};\n";
  out << "static " << class_name << " " << var_name << " = " << class_name << "();\n";

  if (std::find(includes.begin(), includes.end(), clazz->header_file()) ==
      includes.end())
    includes.push_back(clazz->header_file());
  classes += "  &" + var_name + ",\n";
}

static void GenerateNamespace(std::ostream &out,
                              Namespace const *ns,
                              std::string &namespaces,
                              std::vector<std::string> &includes) {
  out << "\n";
  out << "using namespace " << ns->name() << ";\n\n";

  std::string nested_namespaces = "static Namespace *__NS_" + ns->name() + " = {\n";
  for (size_t i = 0; i < ns->GetNumNamespaces(); i++) {
    GenerateNamespace(out, ns->GetNamespaceAt(i), nested_namespaces, includes);
  }
  std::string classes = "static Class *__C_" + ns->name() + "[] = {\n";
  for (size_t i = 0; i < ns->GetNumClasses(); i++) {
    Class *clazz = ns->GetClassAt(i);
    GenerateClass(out, clazz, nullptr, classes, includes);
  }
  nested_namespaces += "  nullptr };\n";
  classes += "  nullptr };\n";

  if (ns->GetNumNamespaces() > 0) {
    out << nested_namespaces << "\n";
  }
  out << classes << "\n";
  out << "static Namespace __ns_" << ns->name() << " =\n";
  out << "    Namespace(\"" << ns->name() << "\",\n";
  out << "              __C_" << ns->name() << ",\n";
  if (ns->GetNumNamespaces() > 0) {
    out << "              _NS_" << ns->name() << ");\n";
  } else {
    out << "              nullptr);\n";
  }
  namespaces += "  &__ns_" + ns->name() + ",\n";
}

void GeneratePackage(char const *path, Package const *pkg) {
  std::stringstream out;
  out << "#include \"rfl/reflected.h\"\n";
  out << "using namespace rfl;\n";

  std::vector<std::string> includes;
  std::string namespaces = "static Namespace *__NS__[] = {\n";
  for (size_t i = 0; i < pkg->GetNumNamespaces(); i++) {
    Namespace *ns = pkg->GetNamespaceAt(i);
    GenerateNamespace(out, ns, namespaces, includes);
  }
  namespaces += "  nullptr\n};\n";
  out << namespaces << "\n";

  out << "static Package __package__ = Package(\"" << pkg->name() << "\", \""
      << pkg->version() << "\", __NS__);\n\n";


  out << "extern \"C\" {\n";
  out << "rfl::Package const *LoadPackage() {\n";
  out << "  return &__package__;\n";
  out << "}\n";
  out << "} // extern \"C\"\n";
  out << "\n";

  std::ofstream file_out;
  file_out.open(path, std::ios_base::out);
  for (std::string const &include : includes) {
    file_out << "#include \"" << include << "\"\n";
  }
  file_out << "\n";
  file_out << out.str();
}
#if 0

class TestBaseObjectClass : public Class {
public:
  enum class Properties {
    int_value,
    float_value,
    ptr_value,
    cptr_value,
    ref_value,
    cfref_value,
    int4_array,
    NUM_PROPERTIES
  };
  TestBaseObjectClass()
      : Class(
            "TestBaseObject",
            Annotation(
                "class:name = \"Test Object Base\"",
                "/home/feed57005/git/librfl/rfl-scan/test/test_annotations.cc",
                17),
            __P___c_TestBaseObject,
            nullptr,
            nullptr) {}

  virtual void *CreateInstance() { return new TestBaseObject(); } 
  virtual void ReleaseInstance(void *) {}
};
#endif
} // namespace rfl
