// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __RFL_REFLECTED__
#define __RFL_REFLECTED__

#include "rfl/rfl_export.h"
#include "rfl/types.h"
#include "rfl/type_info.h"
#include "rfl/any_var.h"

#include <string>
#include <vector>
#include <map>

// non copyable
// string constants
// perfect hashes
// ? dynamic and static variants

namespace rfl {

class Namespace;
typedef std::vector<Namespace *> Namespaces;

class Enum;
typedef std::vector<Enum *> Enums;

class Class;
typedef std::vector<Class *> Classes;

class Field;
typedef std::vector<Field *> Fields;

class Argument;
typedef std::vector<Argument *> Arguments;

class Method;
typedef std::vector<Method *> Methods;

class PackageFile;
typedef std::vector<PackageFile *> PackageFiles;

class Package;

class RFL_EXPORT Annotation {
public:
  Annotation();

  Annotation(Annotation const &x);
  Annotation &operator= (Annotation const &x);

  void AddEntry(std::string const &key, std::string const &value);
  char const *GetEntry(std::string const &key) const;

  std::string const &kind() const { return kind_; }
  void set_kind(std::string const &kind) { kind_ = kind; }

private:
  std::string kind_;

  typedef std::map<std::string, std::string> EntryMap;
  EntryMap entries_;
};

class RFL_EXPORT TypeQualifier {
public:
  TypeQualifier();
  TypeQualifier(TypeQualifier const &x);
  TypeQualifier &operator=(TypeQualifier const &x);

  bool is_pointer() const { return is_pointer_; }
  void set_is_pointer(bool is) { is_pointer_ = is; }

  bool is_ref() const { return is_ref_; }
  void set_is_ref(bool is) { is_ref_ = is; }

  bool is_pod() const { return is_pod_; }
  void set_is_pod(bool is) { is_pod_ = is; }

  bool is_array() const { return is_array_; }
  void set_is_array(bool is) { is_array_ = is; }

  bool is_const() const { return is_const_; }
  void set_is_const(bool is) { is_const_ = is; }

  bool is_mutable() const { return is_mutable_; }
  void set_is_mutable(bool is) { is_mutable_ = is; }

  bool is_volatile() const { return is_volatile_; }
  void set_is_volatile(bool is) { is_volatile_ = is; }

  bool is_restrict() const { return is_restrict_; }
  void set_is_restrict(bool is) { is_restrict_ = is; }

private:
  bool is_pointer_ : 1;
  bool is_pod_ : 1;
  bool is_array_ : 1;
  bool is_const_ : 1;
  bool is_ref_ : 1;
  bool is_mutable_ : 1;
  bool is_volatile_ : 1;
  bool is_restrict_ : 1;
};

class RFL_EXPORT Reflected {
public:
  Reflected() {}
  Reflected(std::string const &name);
  Reflected(std::string const &name, Annotation const &anno);

  std::string const &name() const { return name_; }
  Annotation const &annotation() const { return annotation_; }

private:
  std::string name_;
  Annotation annotation_;
};

class RFL_EXPORT Field : public Reflected {
public:
  Field(std::string const &name,
        std::string const &type,
        uint32 offset,
        TypeQualifier const &type_qualifier,
        Annotation const &anno);

  Class *parent_class() const { return class_; }

  std::string const &type() const { return type_; }

  uint32 offset() const { return offset_ ;}

  TypeQualifier const &type_qualifier() const { return type_qualifier_; }

private:
  std::string type_;
  uint32 offset_;
  TypeQualifier type_qualifier_;
  Class *class_;

  friend class Class;
  void set_parent_class(Class *clazz);
};

class RFL_EXPORT Enum : public Reflected {
public:
  struct RFL_EXPORT EnumItem {
    long value_;
    std::string id_;
    std::string name_;
  };
public:
  Enum(std::string const &name,
       std::string const &type,
       PackageFile *pkg_file,
       Annotation const &anno,
       Namespace *ns,
       Class *parent);

  Namespace *enum_namespace() const { return namespace_; }
  Class *parent_class() const { return parent_class_; }

  void AddEnumItem(EnumItem const &item);
  void RemoveEnumItem(EnumItem const &item);
  EnumItem const &GetEnumItemAt(size_t idx) const;
  size_t GetNumEnumItems() const;

  std::string const &type() const { return type_; }
  PackageFile *package_file() const;

private:
  Namespace *namespace_;
  Class *parent_class_;
  std::string type_;
  std::vector<EnumItem> items_;
  PackageFile *pkg_file_;
};

class RFL_EXPORT Argument : public Reflected {
public:
  enum Kind {
    kReturn_Kind,
    kInput_Kind,
    kOutput_Kind,
    kInOut_Kind
  };
  Argument() {}
  Argument(std::string name,
           Kind kind,
           std::string const &type,
           Annotation const &anno)
      : Reflected(name, anno), kind_(kind), type_(type) {}

  Kind kind() const { return kind_; }
  std::string const &type() const { return type_; }

private:
  Kind kind_;
  std::string type_;
};

class RFL_EXPORT Method : public Reflected {
public:
  Method() {}
  Method(std::string const &name, Annotation const &anno)
    : Reflected(name, anno) {}

  Class *parent_class() const { return class_; }

  void AddArgument(Argument *arg);
  void RemoveArgument(Argument *arg);

  Argument *GetArgumentAt(size_t idx) const;
  size_t GetNumArguments() const;

private:
  friend class Class;
  void set_parent_class(Class *klass) { class_ = klass; }
  Class *class_;
  Arguments arguments_;
};

class RFL_EXPORT EnumContainer {
public:
  EnumContainer();
  void AddEnum(Enum *e);
  void RemoveEnum(Enum *e);
  Enum *FindEnum(char const *enum_name) const;
  size_t GetNumEnums() const;
  Enum *GetEnumAt(size_t idx) const;
private:
  Enums enums_;
};

class RFL_EXPORT Class : public Reflected, public EnumContainer {
public:
  Class(std::string const &name,
        PackageFile *pkg_file,
        Annotation const &anno,
        Class *super = nullptr,
        Field **props = nullptr,
        Class **nested = nullptr
        );

  Namespace *class_namespace() const { return namespace_; }
  Class *parent_class() const { return parent_; }
  Class *super_class() const { return super_; }
  std::string const &header_file() const;
  PackageFile *package_file() const;

  void AddField(Field *prop);
  void RemoveField(Field *prop);
  size_t GetNumFields() const;
  Field *GetFieldAt(size_t idx) const;
  Field *FindField(char const *name) const;

  void AddMethod(Method *method);
  void RemoveMethod(Method *method);
  size_t GetNumMethods() const;
  Method *GetMethodAt(size_t idx) const;
  Method *FindMethod(char const *name) const;

  void AddClass(Class *klass);
  void RemoveClass(Class *klass);
  Class *FindClass(char const *class_name) const;
  size_t GetNumClasses() const;
  Class *GetClassAt(size_t idx) const;

  uint32 order() const;
  void set_order(uint32 order);

private:
  friend class Namespace;
  void set_class_namespace(Namespace *ns) { namespace_ = ns; }
  void set_parent_class(Class *parent) { parent_ = parent; }

private:
  Namespace *namespace_;
  Class *parent_;
  Class *super_;
  Fields fields_;
  Classes classes_;
  Methods methods_;
  PackageFile *pkg_file_;
  uint32 order_;
};

class RFL_EXPORT Namespace : public Reflected, public EnumContainer {
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

class RFL_EXPORT PackageFile : public EnumContainer {
public:
  PackageFile(std::string const &path);

  std::string const &source_path() const;
  std::string filename() const;

  bool is_dependency() const;
  void set_is_dependecy(bool is);

  void AddClass(Class *klass);
  void RemoveClass(Class *klass);
  size_t GetNumClasses() const;
  Class *GetClassAt(size_t idx) const;

private:
  std::string source_path_;
  bool is_dependency_;
  Classes classes_;
};

class RFL_EXPORT Package : public Namespace {
public:
  Package(std::string const &name,
          std::string const &version,
          Namespace **nested = nullptr);

  void AddImport(std::string const &import);
  std::string const &GetImportAt(int idx) const;
  int GetImportNum() const;

  void AddLibrary(std::string const &import);
  std::string const &GetLibraryAt(int idx) const;
  int GetLibraryNum() const;

  std::string const &version() const { return version_; }

  PackageFile *GetOrCreatePackageFile(std::string const &path);

  void AddPackageFile(PackageFile *pkg_file);
  void RemovePackageFile(PackageFile *pkg_file);
  size_t GetNumPackageFiles() const;
  PackageFile *GetPackageFileAt(size_t idx) const;

private:
  std::vector<std::string> imports_;
  std::vector<std::string> libs_;
  std::string version_;
  PackageFiles files_;
};

class RFL_EXPORT PackageManifest {
public:
  bool Load(char const *filename);
  bool Save(char const *filename);
  char const *GetEntry(char const *entry) const;
  void SetEntry(char const *key, char const *value);
private:
  typedef std::map<std::string, std::string> EntryMap;
  EntryMap entry_map_;
};

typedef Package const *(*LoadPackageFunc)();

} // namespace rfl

#endif /* __RFL_REFLECTED__ */
