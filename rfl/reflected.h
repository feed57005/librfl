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
  typedef std::map<std::string, std::string> EntryMap;
public:
  typedef EntryMap::value_type Entry;

  Annotation();

  Annotation(Annotation const &x);
  Annotation &operator= (Annotation const &x);

  void AddEntry(char const *key, char const *value);
  char const *GetEntry(char const *key) const;

  template <class T>
  void EnumerateEntries(T &enumerator) const {
    for (Entry const &entry : entries_) {
      enumerator(entry);
    }
  }

  char const *kind() const;
  void set_kind(char const *kind);

private:
  std::string kind_;
  EntryMap entries_;
};

template <class T>
class AnnotationEntryFilter {
public:
  AnnotationEntryFilter(T &enumerator, std::string const &filter)
      : enumerator_(enumerator), filter_(filter) {}

  void operator()(Annotation::Entry const &e) {
    if (e.first.find(filter_, 0) != std::string::npos) {
      enumerator_(e);
    }
  }

private:
  T &enumerator_;
  std::string const filter_;
};

// TODO source file
class RFL_EXPORT TypeRef {
public:
	enum Kind {
		kInvalid_Kind = 0,
		kSystem_Kind,
		kEnum_Kind,
		kClass_Kind,
	};

	TypeRef();
	TypeRef(TypeRef const &x);
	TypeRef &operator=(TypeRef const &x);
	~TypeRef();

	Kind kind() const;

	char const *type_name() const;
	void set_type_name(char const *name);

	Enum *enum_type() const;
	void set_enum_type(Enum *enm);

	Class *class_type() const;
	void set_class_type(Class *klass);

private:
	Kind kind_;
	std::string type_name_;
	union {
		Class *class_type_;
		Enum *enum_type_;
	};
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
  Reflected(char const *name);
  Reflected(char const *name, Annotation const &anno);

  char const *name() const;
  Annotation const &annotation() const;

private:
  std::string name_;
  Annotation annotation_;
};

class RFL_EXPORT Field : public Reflected {
public:
  Field(char const *name,
        TypeRef const &typeref,
        uint32 offset,
        TypeQualifier const &type_qualifier,
        Annotation const &anno);

  Class *parent_class() const;
  TypeRef const &type_ref() const;
  uint32 offset() const;
  TypeQualifier const &type_qualifier() const;

private:
  TypeRef type_ref_;
  uint32 offset_;
  TypeQualifier type_qualifier_;
  Class *class_;

  friend class Class;
  void set_parent_class(Class *clazz);
};

class RFL_EXPORT EnumItem {
public:
	EnumItem();
	EnumItem(long value, char const *id, char const *name);
	EnumItem(EnumItem const &x);
	EnumItem &operator=(EnumItem const &x);

  long value() const;
  void set_value(long value);

  char const *id() const;
  void set_id(char const *id);

  char const *name() const;
  void set_name(char const *name);

private:
  long value_;
  std::string id_;
  std::string name_;
};

class RFL_EXPORT Enum : public Reflected {
public:
  Enum(char const *name,
       char const *type,
       PackageFile *pkg_file,
       Annotation const &anno,
       Namespace *ns,
       Class *parent);

  Namespace *enum_namespace() const;
  Class *parent_class() const;

  void AddEnumItem(EnumItem const &item);
  void RemoveEnumItem(EnumItem const &item);
  EnumItem const &GetEnumItemAt(size_t idx) const;
  size_t GetNumEnumItems() const;

  char const *type() const;
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
  Argument();
  Argument(char const *name,
           Kind kind,
           char const *type,
           Annotation const &anno);

  Kind kind() const;
  char const *type() const;

private:
  Kind kind_;
  std::string type_;
};

class RFL_EXPORT Method : public Reflected {
public:
  Method();
  Method(char const *name, Annotation const &anno);

  Class *parent_class() const;

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
  Class(char const *name,
        PackageFile *pkg_file,
        Annotation const &anno,
        Class *super = nullptr,
        Field **props = nullptr,
        Class **nested = nullptr
        );

  Namespace *class_namespace() const;
  Class *parent_class() const;
  Class *super_class() const;
  char const *header_file() const;
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

  uint32 base_class_offset() const;
  void set_base_class_offset(uint32 offset);

private:
  friend class Namespace;
  void set_class_namespace(Namespace *ns);
  void set_parent_class(Class *parent);

private:
  Namespace *namespace_;
  Class *parent_;
  Class *super_;
  Fields fields_;
  Classes classes_;
  Methods methods_;
  PackageFile *pkg_file_;
  uint32 order_;
  uint32 base_class_offset_;
};

class RFL_EXPORT Namespace : public Reflected, public EnumContainer {
public:
  Namespace(char const *name,
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

  Namespace *parent_namespace() const;

private:
  void set_parent_namespace(Namespace *ns);
  Namespace *parent_namespace_;
  Classes classes_;
  Namespaces namespaces_;
};

class RFL_EXPORT PackageFile : public EnumContainer {
public:
  PackageFile(char const *path);

  char const *source_path() const;
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
  Package(char const *name,
          char const *version,
          Namespace **nested = nullptr);

  void AddImport(char const *import);
  char const *GetImportAt(int idx) const;
  int GetImportNum() const;

  void AddLibrary(char const *import);
  char const *GetLibraryAt(int idx) const;
  int GetLibraryNum() const;

  char const *version() const;

  PackageFile *GetOrCreatePackageFile(char const *path);

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
