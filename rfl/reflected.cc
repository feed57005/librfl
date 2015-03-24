#include "rfl/reflected.h"
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <fstream>
#include <sstream>

namespace rfl {

Annotation::Annotation(std::string const &value, char const *file, int line)
  : value_(value), file_(file), line_(line) {}

Annotation::Annotation(Annotation const &x)
  : value_(x.value_), file_(x.file_), line_(x.line_), entries_(x.entries_) {
  }

Annotation &Annotation::operator= (Annotation const &x) {
  value_ = x.value_;
  file_ = x.file_;
  line_ = x.line_;
  entries_ = x.entries_;
      std::cout << "copys "<< x.entries_.size()<< " " << entries_.size() << std::endl;
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
