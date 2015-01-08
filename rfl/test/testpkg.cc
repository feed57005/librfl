#include "rfl/reflected.h"
#include "rfl/native_library.h"
#include "rfl/reflected_io.h"
#include "rfl-scan/test/test_annotations.h"

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
  Property *float_prop = klass->FindProperty("float_value_");
  void *instance = klass->CreateInstance();
  float *float_value = (float *) ((char *) instance + (size_t) float_prop->offset()/ 8);
  *float_value = 666.666;
  test::TestBaseObject *obj = (test::TestBaseObject *) instance;
  std::cout << obj->int_value() << " " << obj->float_value() <<  std::endl;
  //DumpPackage(std::cout, pkg);
  //GeneratePackage("xxx", pkg);
  UnloadNativeLibrary(lib);
  return 0;
}
