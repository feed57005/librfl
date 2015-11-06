// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "example/object.h"
#include "example/package_manifest.h"
#include "example/property.h"

#include <iostream>

namespace example {

ObjectClassInstance::ObjectClassInstance(TypeId id,
                                         ObjectClass *klass,
                                         ObjectClassInstance *parent)
    : TypeClassInstance(id, klass), parent_instance_(parent) {}

ObjectClassInstance::~ObjectClassInstance() {}

PropertySpec *ObjectClassInstance::GetPropertySpec(char const *name) const {
  PropertySpecMap::const_iterator it = properties_.find(std::string(name));
  if (it != properties_.end())
    return it->second;
  return nullptr;
}

Method *ObjectClassInstance::GetMethod(char const *name) const {
  MethodMap::const_iterator it = methods_.find(std::string(name));
  if (it != methods_.end())
    return it->second;
  return nullptr;
}

void ObjectClassInstance::AddPropertySpec(PropertySpec *prop) {
  properties_.insert(std::make_pair(prop->name(), prop));
}

void ObjectClassInstance::AddMethod(Method *method) {
  methods_.insert(std::make_pair(method->name(), method));
}

////////////////////////////////////////////////////////////////////////////////

bool Object::AttachClass(ObjectClass *klass) {
  object_class_ = klass;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

TypeId ObjectClass::ID = 0;

ObjectClass::ObjectClass(char const *name, TypeId parent)
    : TypeClass(name),
      parent_class_id_(parent),
      class_name_(name) {}

ObjectClass::~ObjectClass() {}

Object *ObjectClass::CreateInstance() {
  return nullptr;
}

void ObjectClass::ReleaseInstance(Object *) {}

bool ObjectClass::InitInstance(Object *instance) {
  if (!instance)
    return false;
  instance->AttachClass(this);
  return true;
}

bool ObjectClass::InitType(TypeRepository *repo) {
  return true;
}

PropertySpec *ObjectClass::FindPropertySpec(char const *name) const {
  ObjectClassInstance const *inst = class_instance();
  while (inst) {
    PropertySpec *prop = inst->GetPropertySpec(name);
    if (prop)
      return prop;
    inst = inst->parent_instance();
  }
  return nullptr;
}

Method *ObjectClass::FindMethod(char const *name) const {
  ObjectClassInstance *inst = class_instance();
  while (inst) {
    Method *method = inst->GetMethod(name);
    if (method)
      return method;
    inst = inst->parent_instance();
  }
  return nullptr;
}

void ObjectClass::InitObjectProperties(Object *) {}

////////////////////////////////////////////////////////////////////////////////

Method::Method(char const *name,
               char const *human_name,
               TypeId cid,
               CallDesc *desc)
    : name_(name), human_name_(human_name), class_id_(cid), call_desc_(desc) {}

} // namespace example
