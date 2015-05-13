#include "object.h"
#include <iostream>

std::ostream &operator<<(std::ostream &out, rfl::AnyVar const &value) {
  rfl::TypeInfo type = value.GetType();
  if (type == rfl::TypeInfoOf<int>()) {
    out << value.Cast<int>();
  } else if (type == rfl::TypeInfoOf<float>()) {
    out << value.Cast<float>();
  } else if (type == rfl::TypeInfoOf<long>()) {
    out << value.Cast<long>();
  } else if (type == rfl::TypeInfoOf<std::string>()) {
    out << value.Cast<std::string>();
  } else {
    out << type.GetName();
  }
  return out;
}

rfl::AnyVar get(test::Object *instance, test::Property *prop) {
  rfl::AnyVar var = prop->default_value();
  rfl::TypeInfo type = var.GetType();
  if (type == rfl::TypeInfoOf<int>()) {
    int &val = var.Cast<int>();
    val = prop->Get<int>(instance);
  } else if (type == rfl::TypeInfoOf<float>()) {
    float &val = var.Cast<float>();
    val = prop->Get<float>(instance);
  } else if (type == rfl::TypeInfoOf<std::string>()) {
    std::string &val = var.Cast<std::string>();
    val = prop->Get<std::string>(instance);
  }
  return var;
}

struct PropEnum {
  PropEnum(test::Object *instance) : instance_(instance) {}
  void operator()(test::Property *prop) const {
    rfl::AnyVar val = get(instance_, prop);
    std::cout << prop->name() << " (" << prop->default_value() << ") = " << val << std::endl;
  }
  test::Object *instance_;
};

struct MethodEnum {
  MethodEnum(test::Object *instance) : instance_(instance) {}
  void operator()(test::Method *method) const {
    std::cout << method->name() << " - " << method->human_name() << std::endl;
  }
  test::Object *instance_;
};
int main(int argc, char **argv) {
  using namespace test;
  if (argc < 4) {
    std::cout << argv[0] << " <path> <package> <class>" << std::endl;
    return -1;
  }

  ClassRepository *repo = ClassRepository::GetSharedInstance();
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

  klass->EnumerateProperties(PropEnum(obj));
  klass->EnumerateMethods(MethodEnum(obj));

  return 0;
}
