#ifndef __RFL_REFLECTED__
#define __RFL_REFLECTED__

#include "rfl/rfl_export.h"
#include "rfl/types.h"
#include "rfl/type_info.h"
#include "rfl/any_var.h"

#include <string>
#include <vector>

// non copyable
// string constants
// perfect hashes
// ? dynamic and static variants

namespace rfl {

class Namespace;
typedef std::vector<Namespace *> Namespaces;

class Class;
typedef std::vector<Class *> Classes;

class Property;
typedef std::vector<Property *> Properties;
class Package;

class RFL_EXPORT Annotation {
public:
  std::string value_;
  char const *file_;
  int line_;

  Annotation() {}
  Annotation(std::string const &value, char const *file, int line);
  Annotation(Annotation const &x);
  Annotation &operator= (Annotation const &x);
};

class RFL_EXPORT Reflected {
public:
  Reflected() {}
  Reflected(std::string const &name);
  Reflected(std::string const &name, Annotation const &anno);
  Reflected(Reflected const &x);
  Reflected &operator=(Reflected const &x);

  std::string const &name() const { return name_; }
  Annotation const &annotation() const { return annotation_; }

private:
  std::string name_;
  Annotation annotation_;
};

// XXX rename to Field
class RFL_EXPORT Property : public Reflected {
public:
  Property() {}

  Property(std::string const &name,
           std::string const &type,
           uint32 offset,
           Annotation const &anno);

  Property(Property const &x);

  Property &operator=(Property const &x);

  Class *parent_class() const { return class_; }
  std::string const &type() const { return type_; }

  uint32 offset() const { return offset_ ;}
private:
  std::string type_;
  uint32 offset_;
  friend class Class;
  void set_parent_class(Class *clazz) { class_ = clazz; }
  Class *class_;
};

class RFL_EXPORT Class : public Reflected {
public:
  Class() {}

  Class(std::string const &name,
        std::string const &header_file,
        Annotation const &anno,
        Property **props = nullptr,
        Class **nested = nullptr,
        Class *super = nullptr);

  Class(Class const &x);

  Class &operator=(Class const &x);

  Namespace *class_namespace() const { return namespace_; }
  Class *parent_class() const { return parent_; }
  Class *super_class() const { return super_; }

  void AddProperty(Property *prop);
  void RemoveProperty(Property *prop);
  size_t GetNumProperties() const;
  Property *GetPropertyAt(size_t idx) const;
  Property *FindProperty(char const *name) const;

  void AddClass(Class *klass);
  void RemoveClass(Class *klass);
  Class *FindClass(char const *class_name) const;
  size_t GetNumClasses() const;
  Class *GetClassAt(size_t idx) const;

  std::string const &header_file() const { return header_file_; }

  virtual void *CreateInstance() { return nullptr; }

private:
  friend class Namespace;
  void set_class_namespace(Namespace *ns) { namespace_ = ns; }
  void set_parent_class(Class *parent) { parent_ = parent; }

private:
  Namespace *namespace_;
  Class *parent_;
  Class *super_;
  Properties properties_;
  Classes classes_;
  std::string header_file_;
};

class RFL_EXPORT Namespace : public Reflected {
public:
  Namespace(std::string const &name,
            Class **classes = nullptr,
            Namespace **namespaces = nullptr);

  void AddClass(Class *klass);
  void RemoveClass(Class *klass);
  Class *FindClass(char const *class_name) const;
  size_t GetNumClasses() const;
  Class *GetClassAt(size_t idx) const;

  void AddNamespace(Namespace *ns);
  void RemoveNamespace(Namespace *ns);
  Namespace *FindNamespace(char const *ns) const;
  size_t GetNumNamespaces() const;
  Namespace *GetNamespaceAt(size_t idx) const;

  Namespace *parent_namespace() const { return parent_namespace_; }
private:
  void set_parent_namespace(Namespace *ns) { parent_namespace_ = ns; }
  Namespace *parent_namespace_;
  Classes classes_;
  Namespaces namespaces_;
};

class RFL_EXPORT Package : public Namespace {
public:
  Package(std::string const &name,
          std::string const &version,
          Namespace **nested = nullptr);

  void AddImport(Package *pkg);
  std::string const &version() const { return version_; }

private:
  std::vector<Package *> imports_;
  std::string version_;
};

typedef Package const *(*LoadPackageFunc)();

} // namespace rfl

#endif /* __RFL_REFLECTED__ */