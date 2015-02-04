#include "rfl/native_library.h"

#if OS_POSIX
#include <dlfcn.h>
#endif

namespace rfl {

#if OS_POSIX
NativeLibrary LoadNativeLibrary(char const *path, std::string *err) {
  void *dl = dlopen(path, RTLD_LAZY);
  if (!dl && err) {
    *err = dlerror();
  }
  return dl;
}

void UnloadNativeLibrary(NativeLibrary nl) {
  int ret = dlclose(nl);
}

void *GetFunctionPointerFromNativeLibrary(NativeLibrary nl, char const *func) {
  return dlsym(nl, func);
}

#endif
} // namespace rfl