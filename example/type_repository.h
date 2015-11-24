// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __EXAMPLE_CLASS_REPOSITORY_H__
#define __EXAMPLE_CLASS_REPOSITORY_H__

#include "example/example_export.h"
#include "example/type_class.h"

#include "rfl/native_library.h"

#include <vector>
#include <string>
#include <map>

namespace example {

class ObjectClass;
class EnumClass;
class Package;
class Object;

class EXAMPLE_EXPORT TypeRepository {
public:
  static TypeRepository *instance();

  Package *LoadPackage(char const *path, char const *pkgname);

  bool RegisterObjectClass(ObjectClass *klass);
  Object *CreateObject(ObjectClass *klass);
  bool UnregisterObjectClass(ObjectClass *klass);

  bool RegisterEnumClass(EnumClass *enm);
  bool UnregisterEnumClass(EnumClass *enm);

  ObjectClass *GetClassByName(char const *name);
  EnumClass *GetEnumByName(char const *name);

private:
  TypeId GetNextTypeId();

private:
  typedef std::map<std::string, TypeClassInstance *> ClassNameMap;
  typedef std::map<TypeId, TypeClassInstance *> ClassInstanceMap;

  ClassInstanceMap instance_map_;
  ClassNameMap name_map_;

  std::vector<Package *> loaded_packages_;
};

typedef Package *(*RegisterPackage)();

} // namespace example

#endif /* __EXAMPLE_CLASS_REPOSITORY_H__ */
