#include "object.h"
#include <iostream>

namespace test {

void ClassInstance::AddProperty(Property *prop) {
  properties_.insert(std::make_pair(prop->name(), prop));
}

Property *ClassInstance::GetProperty(char const *name) const {
  PropertyMap::const_iterator it = properties_.find(std::string(name));
  if (it != properties_.end())
    return it->second;
  return nullptr;
}


// static
ClassRepository *ClassRepository::GetSharedInstance() {
  static ClassRepository *instance = nullptr;
  if (instance == nullptr) {
    instance = new ClassRepository();
  }
  return instance;
}

bool ClassRepository::RegisterClass(ObjectClass *klass) {
  static ClassId last_type_id = 0;
  ClassInstance *parent = nullptr;
  ClassInstanceMap::const_iterator it =
      instance_map_.find(klass->parent_class_id());
  if (it != instance_map_.end()) {
    parent = it->second;
  }
  ClassInstance *instance = new ClassInstance();
  instance->class_id_ = ++last_type_id;
  instance->object_class_ = klass;
  instance->parent_class_instance_ = parent;

  if (!klass->InitClassInstance(instance)) {
    return false;
  }
  if (!klass->InitClassProperties(instance)) {
    return false;
  }
  instance_map_.insert(std::make_pair(instance->class_id_, instance));
  name_map_.insert(std::make_pair(std::string(klass->class_name()), instance));
  return true;
}

bool ClassRepository::UnregisterClass(ObjectClass *klass) {
  // TODO
  return true;
}

ObjectClass *ClassRepository::GetClassByName(char const *name) {
  ClassNameMap::const_iterator it = name_map_.find(std::string(name));
  if (it != name_map_.end())
    return it->second->object_class_;
  return nullptr;
}

bool ClassRepository::LoadPackage(char const *pkgname) {
  std::string err;
  std::string path = pkgname;
  rfl::NativeLibrary lib = rfl::LoadNativeLibrary(path.c_str(), &err);
  if (!lib) {
    std::cerr << err << std::endl;
    return false;
  }
  RegisterPackage reg_fnc =
      (RegisterPackage)rfl::GetFunctionPointerFromNativeLibrary(lib,
                                                                "LoadPackage");
  if (!reg_fnc) {
    std::cerr << pkgname << " is not package, missing 'LoadPackage'" << std::endl;
    rfl::UnloadNativeLibrary(lib);
    return false;
  }
  reg_fnc();
  package_libs_.push_back(lib);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Object::AttachClass(ObjectClass *klass) {
  object_class_ = klass;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

ClassId ObjectClass::ID = 0;

ObjectClass::ObjectClass(char const *name, ClassId parent)
    : class_instance_(nullptr),
      parent_class_id_(parent),
      class_name_(name) {
}

Object *ObjectClass::CreateInstance() {
  return nullptr;
}

void ObjectClass::ReleaseInstance(Object *instance) {
}

bool ObjectClass::InitInstance(Object *instance) {
  if (!instance)
    return false;
  instance->AttachClass(this);
  return true;
}

bool ObjectClass::InitClassInstance(ClassInstance *instance) {
  class_instance_ = instance;
  return true;
}

bool ObjectClass::InitClassProperties(ClassInstance *instance) {
  return true;
}

Property *ObjectClass::FindProperty(char const *name) const {
  ClassInstance *inst = class_instance_;
  while (inst) {
    Property *prop = inst->GetProperty(name);
    if (prop)
      return prop;
    inst = inst->parent_class_instance_;
  }
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

Property::Property(char const *name,
                   char const *human_name,
                   rfl::uint32 offset,
                   ClassId cid,
                   rfl::AnyVar const &default_value)
    : name_(name), human_name_(human_name), offset_(offset), class_id_(cid), default_value_(default_value) {
}
} // namespace test
