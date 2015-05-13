#include "object.h"
#include "package_manifest.h"
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

void ClassInstance::AddMethod(Method *method) {
  methods_.insert(std::make_pair(method->name(), method));
}

Method *ClassInstance::GetMethod(char const *name) const {
  MethodMap::const_iterator it = methods_.find(std::string(name));
  if (it != methods_.end()) {
    return it->second;
  }
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

// static
ClassRepository *ClassRepository::GetSharedInstance() {
  static ClassRepository *instance = nullptr;
  if (instance == nullptr) {
    instance = new ClassRepository();
  }
  return instance;
}

TypeId ClassRepository::GetNextTypeId(){
  static TypeId last_type_id = 0;
	return ++last_type_id;
}

bool ClassRepository::RegisterClass(ObjectClass *klass) {
  ClassInstance *parent = nullptr;
  ClassInstanceMap::const_iterator it =
      instance_map_.find(klass->parent_class_id());
  if (it != instance_map_.end()) {
    parent = it->second;
  }
  ClassInstance *instance = new ClassInstance();
  instance->class_id_ = GetNextTypeId();
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

bool ClassRepository::RegisterEnum(Enum *enm) {
  enm->enum_id_ = GetNextTypeId();
  enum_name_map_.insert(std::make_pair(std::string(enm->enum_name_), enm));
  return true;
}

bool ClassRepository::UnregisterEnum(Enum *enm) {
  // TODO
	return true;
}

Enum *ClassRepository::GetEnumByName(char const *name) {
  EnumNameMap::const_iterator it = enum_name_map_.find(std::string(name));
  if (it != enum_name_map_.end())
    return it->second;
  return nullptr;
}

ObjectClass *ClassRepository::GetClassByName(char const *name) {
  ClassNameMap::const_iterator it = name_map_.find(std::string(name));
  if (it != name_map_.end())
    return it->second->object_class_;
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

bool ClassRepository::LoadPackage(char const *path, char const *pkgname) {
  std::string name = pkgname;
  for (LoadedPackage const &loaded : package_libs_) {
    if (loaded.package_name_.compare(name) == 0) {
      return true;
    }
  }
  std::string err;
  std::string manifest_path = path;
  manifest_path += "/";
  manifest_path += pkgname;
  manifest_path += ".ini";

  PackageManifest manifest;
  if (!manifest.Load(manifest_path.c_str())) {
    std::cerr << "Unable to load manifest " << manifest_path << std::endl;
    return false;
  }
  // load package dependencies
  ImportEnumerator imports_enum;
  manifest.Enumerate(imports_enum);
  for (std::string const &import : imports_enum.imports_) {
    if (!LoadPackage(path, import.c_str()))
      return false;
  }

  // load the package library itself
  std::string lib_path = path;
  lib_path += "/";
  lib_path += manifest.GetEntry("package.library");
  std::cout << "Loading package " << lib_path << std::endl;
  rfl::NativeLibrary lib = rfl::LoadNativeLibrary(lib_path.c_str(), &err);
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
  package_libs_.push_back(LoadedPackage(pkgname, lib));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Object::AttachClass(ObjectClass *klass) {
  object_class_ = klass;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

TypeId ObjectClass::ID = 0;

ObjectClass::ObjectClass(char const *name, TypeId parent)
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

Method *ObjectClass::FindMethod(char const *name) const {
  ClassInstance *inst = class_instance_;
  while (inst) {
    Method *method = inst->GetMethod(name);
    if (method)
      return method;
    inst = inst->parent_class_instance_;
  }
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

Method::Method(char const *name,
               char const *human_name,
               TypeId cid,
               CallDesc *desc)
    : name_(name), human_name_(human_name), class_id_(cid), call_desc_(desc) {
}

////////////////////////////////////////////////////////////////////////////////

Property::Property(char const *name,
                   char const *human_name,
                   rfl::uint32 offset,
                   TypeId cid,
                   rfl::AnyVar const &default_value)
    : name_(name), human_name_(human_name), offset_(offset), class_id_(cid), default_value_(default_value) {
}
} // namespace test
