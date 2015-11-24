// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "example/type_repository.h"
#include "example/object.h"
#include "example/enum_class.h"
#include "example/package.h"
#include "example/package_manifest.h"

#include <iostream>

namespace example {

// static
TypeRepository *TypeRepository::instance() {
  static TypeRepository *instance = nullptr;
  if (instance == nullptr) {
    instance = new TypeRepository();
  }
  return instance;
}

TypeId TypeRepository::GetNextTypeId(){
  static TypeId last_type_id = 0;
	return ++last_type_id;
}

bool TypeRepository::RegisterObjectClass(ObjectClass *klass) {
  // TODO check class is not already registered

  ObjectClassInstance *parent = nullptr;
  ClassInstanceMap::const_iterator it =
      instance_map_.find(klass->parent_class_id());
  if (it != instance_map_.end()) {
    parent = (ObjectClassInstance *) it->second;
  }
  ObjectClassInstance *instance =
      new ObjectClassInstance(GetNextTypeId(), klass, parent);

  klass->type_class_instance_ = instance;
  if (!klass->InitType(this)) {
    klass->type_class_instance_ = nullptr;
    delete instance;
    return false;
  }

  instance_map_.insert(std::make_pair(instance->type_id(), instance));
  name_map_.insert(std::make_pair(std::string(klass->type_name()), instance));
  return true;
}

bool TypeRepository::UnregisterObjectClass(ObjectClass *) {
  // TODO
  return true;
}

Object *TypeRepository::CreateObject(ObjectClass *klass) {
  Object *obj = klass->CreateInstance();
  if (!klass->InitInstance(obj)) {
    klass->ReleaseInstance(obj);
    return nullptr;
  }
  return obj;
}

bool TypeRepository::RegisterEnumClass(EnumClass *enm) {
  // TODO check class is not already registered

  TypeClassInstance *instance = new TypeClassInstance(GetNextTypeId(), enm);
  instance_map_.insert(std::make_pair(instance->type_id(), instance));
  name_map_.insert(std::make_pair(std::string(enm->enum_name()), instance));

  enm->type_class_instance_ = instance;
  if (!enm->InitType(this)) {
    delete instance;
    enm->type_class_instance_ = nullptr;
    return false;
  }
  return true;
}

bool TypeRepository::UnregisterEnumClass(EnumClass *) {
  // TODO
	return true;
}

EnumClass *TypeRepository::GetEnumByName(char const *name) {
  ClassNameMap::const_iterator it = name_map_.find(std::string(name));
  if (it != name_map_.end() &&
      it->second->type_class()->GetTypeClassKind() == TypeClass::ENUM_KIND)
    return (EnumClass *)it->second->type_class();
  return nullptr;
}

ObjectClass *TypeRepository::GetClassByName(char const *name) {
  ClassNameMap::const_iterator it = name_map_.find(std::string(name));
  if (it != name_map_.end() &&
      it->second->type_class()->GetTypeClassKind() == TypeClass::OBJECT_KIND)
    return (ObjectClass *)it->second->type_class();
  return nullptr;
}

struct ImportEnumerator {
  std::vector<std::string> imports_;

  void operator()(std::string const &key, std::string const &) {
    size_t pos = key.find("imports.",0);
    if (pos != std::string::npos && pos == 0 && key.size() > 8) {
      imports_.push_back(key.substr(8));
    }
  }
};

Package *TypeRepository::LoadPackage(char const *path, char const *pkgname) {
  std::string name = pkgname;
  for (Package *loaded : loaded_packages_) {
    if (std::string(loaded->name()).compare(name) == 0) {
      return loaded;
    }
  }
  std::string err;
  std::string manifest_path = path;
  manifest_path += "/";
  manifest_path += pkgname;
  manifest_path += "/";
  manifest_path += pkgname;
  manifest_path += ".ini";

  PackageManifest manifest;
  if (!manifest.Load(manifest_path.c_str())) {
    std::cerr << "Unable to load manifest " << manifest_path << std::endl;
    return nullptr;
  }
  // load package dependencies
  ImportEnumerator imports_enum;
  manifest.Enumerate(imports_enum);
  for (std::string const &import : imports_enum.imports_) {
    if (!LoadPackage(path, import.c_str()))
      return nullptr;
  }

  // load the package library itself
  std::string lib_path = path;
  lib_path += "/";
  lib_path += pkgname;
  lib_path += "/";
  lib_path += manifest.GetEntry("package.library");
  std::cout << "Loading package " << lib_path << std::endl;
  rfl::NativeLibrary lib = rfl::LoadNativeLibrary(lib_path.c_str(), &err);
  if (!lib) {
    std::cerr << err << std::endl;
    return nullptr;
  }
  RegisterPackage reg_fnc =
      (RegisterPackage)rfl::GetFunctionPointerFromNativeLibrary(lib,
                                                                "LoadPackage");
  if (!reg_fnc) {
    std::cerr << pkgname << " is not package, missing 'LoadPackage'" << std::endl;
    rfl::UnloadNativeLibrary(lib);
    return nullptr;
  }
  Package *pkg = reg_fnc();
  if (!pkg) {
    std::cerr << pkgname << " did return null package" << std::endl;
    return nullptr;
  }
  pkg->native_lib_ = lib;
  pkg->package_name_ = pkgname;
  loaded_packages_.push_back(pkg);
  return pkg;
}

} // namespace example

EXAMPLE_NAME_INSTANCE(bool)
EXAMPLE_NAME_INSTANCE(int)
EXAMPLE_NAME_INSTANCE(float)
EXAMPLE_NAME_INSTANCE(double)
EXAMPLE_NAME_INSTANCE(long)
EXAMPLE_NAME_INSTANCE(std::string)
EXAMPLE_NAME_INSTANCE(EmptyType)

