#include "rfl/reflected.h"
#include "rfl/native_library.h"
#include "rfl/reflected_io.h"
#include "rfl-scan/test/test_annotations.h"

namespace rfl {


class DynamicObject {
public:
  DynamicObject(void *instance, Class const *klass)
    : instance_(instance), class_(klass) {}

  template <typename T>
  T const &GetProperty(rfl::Property const *prop) const {
    T const *value = (T *) ((char *) instance_ + (size_t) prop->offset()/ 8);
    return *value;
  }

  template <typename T>
  T const *GetProperty(char const *name) const {
    Property *prop = class_->FindProperty(name);
    if (prop == nullptr)
      return nullptr;
    T const *value = (T *) ((char *) instance_ + (size_t) prop->offset()/ 8);
    return value;
  }

  template <typename T>
  void SetProperty(rfl::Property const *prop, T const &new_value) const {
    T *value = (T *) ((char *) instance_ + (size_t) prop->offset()/ 8);
    *value = new_value;
  }

  template <typename T>
  void SetProperty(char const *name, T const &new_value) const {
    Property *prop = class_->FindProperty(name);
    if (prop == nullptr)
      return;
    SetProperty(prop, new_value);
  }

  void *instance() const { return instance_; }
  Class const *instance_class() const { return class_; }

private:
  void *instance_;
  Class const *class_;
};

} // namespace rfl
int main(int argc, char **argv) {

  using namespace rfl;
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " <path_to_rfllib>" << std::endl;
    return -1;
  }

  std::string lib_err;
  NativeLibrary lib = LoadNativeLibrary(argv[1], &lib_err);
  if (!lib) {
    std::cerr << lib_err << std::endl;
  }
  LoadPackageFunc pkg_func =
      (LoadPackageFunc)GetFunctionPointerFromNativeLibrary(lib, "LoadPackage");
  Package const *pkg = pkg_func();
  Namespace const *ns = pkg->FindNamespace("test");
  Class *klass = ns->FindClass("TestBaseObject");
  for (uint32 i = 0; i < klass->GetNumProperties(); i++) {
    Property *prop = klass->GetPropertyAt(i);
    std::cout << prop->name() << ":" << prop->offset() / 8 << std::endl;
  }

  DynamicObject dyn_obj(klass->CreateInstance(), klass);
  Property *float_prop = klass->FindProperty("float_value_");
  dyn_obj.SetProperty(float_prop, float(666.666));
  std::cout << dyn_obj.GetProperty<float>(float_prop) << std::endl;

  test::TestBaseObject *obj = (test::TestBaseObject *) dyn_obj.instance();
  std::cout << obj->int_value() << " " << obj->float_value() <<  std::endl;
  UnloadNativeLibrary(lib);
  return 0;
}
