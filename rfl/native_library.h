#ifndef __RFL_NATIVE_LIBRARY_H__
#define __RFL_NATIVE_LIBRARY_H__

#include "rfl/types.h"
#include "rfl/rfl_export.h"

#include <string>

namespace rfl {

#if OS_POSIX
typedef void* NativeLibrary;
#elif OS_WIN
typedef HMODULE NativeLibrary;
#endif

RFL_EXPORT NativeLibrary
    LoadNativeLibrary(char const *path, std::string *err = NULL);

RFL_EXPORT void UnloadNativeLibrary(NativeLibrary nl);

RFL_EXPORT void *GetFunctionPointerFromNativeLibrary(NativeLibrary nl,
                                                     char const *func);

} // namespace rfl

#endif /* __RFL_NATIVE_LIBRARY_H__ */
