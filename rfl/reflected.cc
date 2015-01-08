#include "rfl/reflected.h"
#include <algorithm>
#include <iostream>
#include <assert.h>
namespace rfl {

Annotation::Annotation(std::string const &value, char const *file, int line)
  : value_(value), file_(file), line_(line) {}

Annotation::Annotation(Annotation const &x)
  : value_(x.value_), file_(x.file_), line_(x.line_) {}

Annotation &Annotation::operator= (Annotation const &x) {
  value_ = x.value_;
  file_ = x.file_;
  line_ = x.line_;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Reflected::Reflected(std::string const &name) : name_(name) {
}

Reflected::Reflected(std::string const &name, Annotation const &anno)
    : name_(name), annotation_(anno) {
}

Reflected::Reflected(Reflected const &x)
    : name_(x.name_), annotation_(x.annotation_) {
}

Reflected &Reflected::operator=(Reflected const &x) {
  name_ = x.name_;
  annotation_ = x.annotation_;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

Property::Property(std::string const &name,
                   std::string const &type,
                   uint32 offset,
                   Annotation const &anno)
    : Reflected(name, anno),
      type_(type),
      offset_(offset),
      class_(nullptr) {
}

Property::Property(Property const &x)
    : Reflected(x),
      type_(x.type_),
      offset_(x.offset_),
      class_(x.class_) {
}

Property &Property::operator=(Property const &x) {
  Reflected::operator=(x);
  type_ = x.type_;
  offset_ = x.offset_;
  class_ = x.class_;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

Class::Class(std::string const &name,
             std::string const &header_file,
             Annotation const &anno,
             Property **props,
             Class **nested,
             Class *super)
    : Reflected(name, anno),
      namespace_(nullptr),
      parent_(nullptr),
      super_(super),
      header_file_(header_file) {
  if (props != nullptr) {
    Property **prop = &props[0];
    while (*prop != nullptr) {
      AddProperty(*prop);
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
}

Class::Class(Class const &x)
    : Reflected(x),
      namespace_(x.namespace_),
      parent_(x.parent_),
      super_(x.super_),
      properties_(x.properties_),
      classes_(x.classes_) {
}

Class &Class::operator=(Class const &x) {
  Reflected::operator=(x);
  namespace_ = x.namespace_;
  parent_ = x.parent_;
  super_ = x.super_;
  properties_ = x.properties_;
  classes_ = x.classes_;
  return *this;
}

void Class::AddProperty(Property *prop) {
  assert(prop != nullptr);
  properties_.push_back(prop);
  prop->set_parent_class(this);
}

void Class::RemoveProperty(Property *prop) {
  std::vector<Property *>::iterator it =
      std::find(properties_.begin(), properties_.end(), prop);
  if (it != properties_.end())
    properties_.erase(it);
  prop->set_parent_class(nullptr);
}

size_t Class::GetNumProperties() const {
  return properties_.size();
}

Property *Class::GetPropertyAt(size_t idx) const {
  return properties_[idx];
}

Property *Class::FindProperty(char const *name) const {
  for (Property *prop : properties_) {
    if (prop->name().compare(std::string(name)) == 0) {
      return prop;
    }
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

////////////////////////////////////////////////////////////////////////////////

Namespace::Namespace(std::string const &name,
                     Class **classes,
                     Namespace **namespaces)
    : Reflected(name) {
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

Package::Package(std::string const &name,
                 std::string const &version,
                 Namespace **nested)
    : Namespace(name, nullptr, nested), version_(version) {
}

void Package::AddImport(Package *pkg) {
  imports_.push_back(pkg);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace rfl
