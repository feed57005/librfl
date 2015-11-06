// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "example/object.h"
#include "example/property.h"
#include "example/type_repository.h"

#include <iostream>

using namespace example;

std::ostream &operator<<(std::ostream &out, AnyVar const &value) {
  TypeInfo type = value.GetType();
  if (type == TypeInfoOf<int>()) {
    out << value.Cast<int>();
  } else if (type == TypeInfoOf<float>()) {
    out << value.Cast<float>();
  } else if (type == TypeInfoOf<long>()) {
    out << value.Cast<long>();
  } else if (type == TypeInfoOf<std::string>()) {
    out << value.Cast<std::string>();
  } else {
    out << type.GetName();
  }
  return out;
}
#if 0
AnyVar get(example::Object *instance, example::PropertySpec *prop) {
  AnyVar var = prop->default_value();
  TypeInfo type = var.GetType();
  if (type == TypeInfoOf<int>()) {
    int &val = var.Cast<int>();
    val = prop->Get<int>(instance);
  } else if (type == TypeInfoOf<float>()) {
    float &val = var.Cast<float>();
    val = prop->Get<float>(instance);
  } else if (type == TypeInfoOf<std::string>()) {
    std::string &val = var.Cast<std::string>();
    val = prop->Get<std::string>(instance);
  }
  return var;
}
#endif
struct PropEnum {
  PropEnum(example::Object *instance) : instance_(instance) {}
  void operator()(example::PropertySpec *prop) const {
    //AnyVar val = get(instance_, prop);
    std::cout << prop->name() << " (" << prop->default_value() << ")" << std::endl;
  }
  example::Object *instance_;
};

struct MethodEnum {
  MethodEnum(example::Object *instance) : instance_(instance) {}
  void operator()(example::Method *method) const {
    std::cout << method->name() << " - " << method->human_name() << std::endl;
  }
  example::Object *instance_;
};

int main(int argc, char **argv) {
  if (argc < 4) {
    std::cout << argv[0] << " <path> <package> <class>" << std::endl;
    return -1;
  }

  TypeRepository *repo = TypeRepository::instance();
  if (!repo->LoadPackage(argv[1], argv[2])) {
    std::cerr << "failed to load package " << argv[1] << std::endl;
    return -1;
  }

  ObjectClass *klass = repo->GetClassByName(argv[3]);
  if (!klass) {
    std::cerr << "failed to find class " << argv[3] << std::endl;
    return -1;
  }

  Object *obj = klass->CreateInstance();
  if (!obj) {
    std::cerr << "failed to create class instance " << argv[3] << std::endl;
    return -1;
  }

  std::cout << "ClassID: " << klass->class_id()
            << " parent ID: " << klass->parent_class_id() << std::endl;

  klass->class_instance()->EnumerateProperties(PropEnum(obj));
  klass->class_instance()->EnumerateMethods(MethodEnum(obj));

  return 0;
}
