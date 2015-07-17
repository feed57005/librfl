// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rfl/reflected.h"

#include <algorithm>
#include <iostream>
#include <assert.h>
#include <fstream>
#include <sstream>

namespace rfl {

Annotation::Annotation() {
}

Annotation::Annotation(Annotation const &x)
    : kind_(x.kind_), entries_(x.entries_) {
}

Annotation &Annotation::operator=(Annotation const &x) {
  kind_ = x.kind_;
  entries_ = x.entries_;
  return *this;
}

void Annotation::AddEntry(std::string const &key, std::string const &value) {
  entries_.insert(std::make_pair(key, value));
}

char const *Annotation::GetEntry(std::string const &key) const {
  EntryMap::const_iterator it = entries_.find(key);
  if (it != entries_.end())
    return it->second.c_str();
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

TypeQualifier::TypeQualifier()
    : is_pointer_(false),
      is_pod_(false),
      is_array_(false),
      is_const_(false),
      is_ref_(false),
      is_mutable_(false),
      is_volatile_(false) {
}

TypeQualifier::TypeQualifier(TypeQualifier const &x)
    : is_pointer_(x.is_pointer_),
      is_pod_(x.is_pod_),
      is_array_(x.is_array_),
      is_const_(x.is_const_),
      is_ref_(x.is_ref_),
      is_mutable_(x.is_mutable_),
      is_volatile_(x.is_volatile_) {
}

TypeQualifier &TypeQualifier::operator=(TypeQualifier const &x) {
  is_pointer_ = x.is_pointer_;
  is_pod_ = x.is_pod_;
  is_array_ = x.is_array_;
  is_const_ = x.is_const_;
  is_ref_ = x.is_ref_;
  is_mutable_ = x.is_mutable_;
  is_volatile_ = x.is_volatile_;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

Reflected::Reflected(std::string const &name) : name_(name) {
}

Reflected::Reflected(std::string const &name, Annotation const &anno)
    : name_(name), annotation_(anno) {
}

////////////////////////////////////////////////////////////////////////////////

Field::Field(std::string const &name,
             std::string const &type,
             uint32 offset,
             TypeQualifier const &type_qualifier,
             Annotation const &anno)
    : Reflected(name, anno),
      type_(type),
      offset_(offset),
      type_qualifier_(type_qualifier),
      class_(nullptr) {
}

void Field::set_parent_class(Class *clazz) {
  class_ = clazz;
}

////////////////////////////////////////////////////////////////////////////////

Enum::Enum(std::string const &name,
           std::string const &type,
           PackageFile *pkg_file,
           Annotation const &anno,
           Namespace *ns,
           Class *parent)
    : Reflected(name, anno),
      namespace_(ns),
      parent_class_(parent),
      type_(type),
      pkg_file_(pkg_file) {
  if (pkg_file_)
    pkg_file_->AddEnum(this);
}

void Enum::AddEnumItem(EnumItem const &item) {
  items_.push_back(item);
}

void Enum::RemoveEnumItem(EnumItem const &item) {
  for (std::vector<EnumItem>::iterator it = items_.begin(); it != items_.end(); ++it) {
    if (it->value_ == item.value_) {
      items_.erase(it);
      return;
    }
  }
}

Enum::EnumItem const &Enum::GetEnumItemAt(size_t idx) const {
  return items_[idx];
}

size_t Enum::GetNumEnumItems() const {
  return items_.size();
}

PackageFile *Enum::package_file() const {
  return pkg_file_;
}

////////////////////////////////////////////////////////////////////////////////

EnumContainer::EnumContainer() {
}

void EnumContainer::AddEnum(Enum *e) {
  enums_.push_back(e);
}

void EnumContainer::RemoveEnum(Enum *e) {
  Enums::iterator it = std::find(enums_.begin(), enums_.end(), e);
  if (it != enums_.end())
    enums_.erase(it);
}

Enum *EnumContainer::FindEnum(char const *enum_name) const {
  std::string const ename(enum_name);
  for (Enum *e : enums_) {
    if (e->name().compare(ename) == 0)
      return e;
  }
  return nullptr;
}

size_t EnumContainer::GetNumEnums() const {
  return enums_.size();
}

Enum *EnumContainer::GetEnumAt(size_t idx) const {
  return enums_[idx];
}

////////////////////////////////////////////////////////////////////////////////

void Method::AddArgument(Argument *arg) {
  arguments_.push_back(arg);
}

void Method::RemoveArgument(Argument *arg) {
  Arguments::iterator it =
      std::find(arguments_.begin(), arguments_.end(), arg);
  if (it != arguments_.end())
    arguments_.erase(it);
}

Argument *Method::GetArgumentAt(size_t idx) const {
  return arguments_[idx];
}

size_t Method::GetNumArguments() const {
  return arguments_.size();
}

////////////////////////////////////////////////////////////////////////////////

Class::Class(std::string const &name,
             PackageFile *pkg_file,
             Annotation const &anno,
             Class *super,
             Field **props,
             Class **nested
             )
    : Reflected(name, anno),
      namespace_(nullptr),
      parent_(nullptr),
      super_(super),
      pkg_file_(pkg_file) {
  if (props != nullptr) {
    Field **prop = &props[0];
    while (*prop != nullptr) {
      AddField(*prop);
      prop++;
    }
  }
  if (nested) {
    Class **clazz = &nested[0];
    while (*clazz != nullptr) {
      AddClass(*clazz);
      clazz++;
    }
  }
  if (pkg_file_) {
    pkg_file_->AddClass(this);
  }
}

void Class::AddField(Field *prop) {
  assert(prop != nullptr);
  fields_.push_back(prop);
  prop->set_parent_class(this);
}

void Class::RemoveField(Field *prop) {
  std::vector<Field *>::iterator it =
      std::find(fields_.begin(), fields_.end(), prop);
  if (it != fields_.end())
    fields_.erase(it);
  prop->set_parent_class(nullptr);
}

size_t Class::GetNumFields() const {
  return fields_.size();
}

Field *Class::GetFieldAt(size_t idx) const {
  return fields_[idx];
}

Field *Class::FindField(char const *name) const {
  for (Field *prop : fields_) {
    if (prop->name().compare(std::string(name)) == 0) {
      return prop;
    }
  }
  return nullptr;
}

void Class::AddMethod(Method *method) {
  methods_.push_back(method);
}

void Class::RemoveMethod(Method *method) {
  Methods::iterator it = std::find(methods_.begin(), methods_.end(), method);
  if (it != methods_.end())
    methods_.erase(it);
}

size_t Class::GetNumMethods() const {
  return methods_.size();
}

Method *Class::GetMethodAt(size_t idx) const {
  return methods_[idx];
}

Method *Class::FindMethod(char const *name) const {
  std::string const mname(name);
  for (Method *m : methods_) {
    if (m->name().compare(mname) == 0)
      return m;
  }
  return nullptr;
}

void Class::AddClass(Class *klass) {
  classes_.push_back(klass);
  klass->set_parent_class(this);
}

void Class::RemoveClass(Class *klass) {
  std::vector<Class *>::iterator it =
      std::find(classes_.begin(), classes_.end(), klass);
  if (it != classes_.end())
    classes_.erase(it);
  klass->set_parent_class(nullptr);
}

Class *Class::FindClass(char const *class_name) const {
  for (Class *klass : classes_) {
    if (klass->name().compare(std::string(class_name)) == 0) {
      return klass;
    }
  }
  return nullptr;
}

size_t Class::GetNumClasses() const {
  return classes_.size();
}

Class *Class::GetClassAt(size_t idx) const {
  return classes_[idx];
}

std::string const &Class::header_file() const {
  return pkg_file_->source_path();
}

PackageFile *Class::package_file() const {
  return pkg_file_;
}

uint32 Class::order() const {
  return order_;
}

void Class::set_order(uint32 order) {
  order_ = order;
}

////////////////////////////////////////////////////////////////////////////////

Namespace::Namespace(std::string const &name,
                     Class **classes,
                     Namespace **namespaces)
    : Reflected(name), parent_namespace_(nullptr) {
  if (classes != nullptr) {
    Class **clazz = &classes[0];
    while (*clazz != nullptr) {
      AddClass(*clazz);
      clazz++;
    }
  }
  if (namespaces) {
    Namespace **ns = &namespaces[0];
    while (*ns != nullptr) {
      AddNamespace(*ns);
      ns++;
    }
  }
}

void Namespace::AddClass(Class *klass) {
  classes_.push_back(klass);
  klass->set_class_namespace(this);
}

void Namespace::RemoveClass(Class *klass) {
  std::vector<Class *>::iterator it =
      std::find(classes_.begin(), classes_.end(), klass);
  if (it != classes_.end())
    classes_.erase(it);
  klass->set_class_namespace(nullptr);
}

Class *Namespace::FindClass(char const *class_name) const {
  for (Class *klass : classes_) {
    if (klass->name().compare(std::string(class_name)) == 0) {
      return klass;
    }
  }
  return nullptr;
}

size_t Namespace::GetNumClasses() const {
  return classes_.size();
}

Class *Namespace::GetClassAt(size_t idx) const {
  return classes_[idx];
}

void Namespace::AddNamespace(Namespace *ns) {
  namespaces_.push_back(ns);
  ns->set_parent_namespace(this);
}

void Namespace::RemoveNamespace(Namespace *ns) {
  std::vector<Namespace *>::iterator it =
      std::find(namespaces_.begin(), namespaces_.end(), ns);
  if (it != namespaces_.end())
    namespaces_.erase(it);
}

Namespace *Namespace::FindNamespace(char const *name) const {
  for (Namespace *ns : namespaces_) {
    if (ns->name().compare(std::string(name)) == 0)
      return ns;
  }
  return nullptr;
}

size_t Namespace::GetNumNamespaces() const {
  return namespaces_.size();
}

Namespace *Namespace::GetNamespaceAt(size_t idx) const {
  return namespaces_[idx];
}

////////////////////////////////////////////////////////////////////////////////

PackageFile::PackageFile(std::string const &path)
    : source_path_(path), is_dependency_(true) {
}

std::string const &PackageFile::source_path() const {
  return source_path_;
}

std::string PackageFile::filename() const {
  size_t pos = source_path_.find_last_of('/');
  if (pos == std::string::npos)
    return source_path_;
  return source_path_.substr(pos+1);
}

void PackageFile::AddClass(Class *klass) {
  classes_.push_back(klass);
}

void PackageFile::RemoveClass(Class *klass) {
  Classes::iterator it = std::find(classes_.begin(), classes_.end(), klass);
  if (it != classes_.end())
    classes_.erase(it);
}

size_t PackageFile::GetNumClasses() const {
  return classes_.size();
}

Class *PackageFile::GetClassAt(size_t idx) const {
  return classes_[idx];
}

bool PackageFile::is_dependency() const {
  return is_dependency_;
}

void PackageFile::set_is_dependecy(bool is) {
  is_dependency_ = is;
}


////////////////////////////////////////////////////////////////////////////////

Package::Package(std::string const &name,
                 std::string const &version,
                 Namespace **nested)
    : Namespace(name, nullptr, nested), version_(version) {
}

void Package::AddImport(std::string const &import) {
  imports_.push_back(import);
}

std::string const &Package::GetImportAt(int idx) const {
  return imports_[idx];
}

int Package::GetImportNum() const {
  return (int) imports_.size();
}

void Package::AddLibrary(std::string const &import) {
  libs_.push_back(import);
}

std::string const &Package::GetLibraryAt(int idx) const {
  return libs_[idx];
}

int Package::GetLibraryNum() const {
  return (int) libs_.size();
}

PackageFile *Package::GetOrCreatePackageFile(std::string const &path) {
  for (PackageFile *file : files_) {
    if (file->source_path().compare(path) == 0)
      return file;
  }
  PackageFile *ret = new PackageFile(path);
  AddPackageFile(ret);
  return ret;
}

void Package::AddPackageFile(PackageFile *pkg_file) {
  files_.push_back(pkg_file);
}

void Package::RemovePackageFile(PackageFile *pkg_file) {
  PackageFiles::iterator it = std::find(files_.begin(), files_.end(), pkg_file);
  if (it != files_.end())
    files_.erase(it);
}

size_t Package::GetNumPackageFiles() const {
  return files_.size();
}

PackageFile *Package::GetPackageFileAt(size_t idx) const {
  return files_[idx];
}

////////////////////////////////////////////////////////////////////////////////

bool PackageManifest::Load(char const *filename) {
  std::ifstream is(filename, std::ios_base::in);
  std::string line;
  std::string current_section;

  while (is.good()) {
    std::getline(is, line);
    if (line.empty()) {
      // Skips the empty line.
      continue;
    }
    if (line[0] == '#' || line[0] == ';') {
      // This line is a comment.
      continue;
    }
    if (line[0] == '[') {
      // It is a section header.
      current_section = line.substr(1);
      size_t end = current_section.rfind(']');
      if (end != std::string::npos)
        current_section.erase(end);
    } else {
      std::string key, value;
      size_t equal = line.find('=');
      if (equal != std::string::npos) {
        key = line.substr(0, equal);
        value = line.substr(equal + 1);
        std::string whole_key = current_section;
        if (!current_section.empty())
          whole_key += ".";
        whole_key += key;
        entry_map_.insert(std::make_pair(whole_key, value));
      }
    }
  }
  is.close();
  return true;
}

bool PackageManifest::Save(char const *filename) {
  std::stringstream out;
  std::vector<std::string> keys;
  for (EntryMap::const_iterator it = entry_map_.begin(); it != entry_map_.end();
       ++it) {
    keys.push_back(it->first);
  }
  std::sort(keys.begin(), keys.end());
  std::string current_section = "";
  for (std::vector<std::string>::const_reverse_iterator it = keys.rbegin(); it != keys.rend(); ++it) {
    std::string const &key = *it;
    size_t pos = key.rfind(".", key.length()-1);
    std::string const &value = entry_map_.find(key)->second;
    if (pos != std::string::npos) {
      std::string section = key.substr(0, pos);
      if (current_section.compare(section) != 0) {
        out << "[" << section << "]\n";
        current_section = section;
      }
      out << "  " << key.substr(pos+1, key.length()-1) << " = " << value << "\n";
    } else {
      out << key << " = " << value << "\n";
    }
  }

  std::ofstream file_out;
  file_out.open(filename, std::ios_base::out);
  file_out << out.str();
  file_out.close();
  return true;
}

char const *PackageManifest::GetEntry(char const *entry) const {
  EntryMap::const_iterator it = entry_map_.find(std::string(entry));
  if (it != entry_map_.end()) {
    return it->second.c_str();
  }
  return nullptr;
}

void PackageManifest::SetEntry(char const *key, char const *value) {
  EntryMap::iterator it = entry_map_.find(std::string(key));
  if (it != entry_map_.end()) {
    it->second = std::string(value);
  } else {
    entry_map_.insert(std::make_pair(std::string(key), std::string(value)));
  }
}

} // namespace rfl
