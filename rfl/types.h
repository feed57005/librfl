#ifndef __LIBRFL_TYPES_H__
#define __LIBRFL_TYPES_H__

#include <cstdint>

namespace rfl {

typedef signed char        int8;
typedef short              int16;
typedef int                int32;
typedef int64_t            int64;

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef uint64_t           uint64;

#if defined(__APPLE__)
#define OS_MACOSX 1
#elif defined(ANDROID)
#define OS_ANDROID 1
#elif defined(__linux__)
#define OS_LINUX 1
#elif defined(_WIN32)
#define OS_WIN 1
#endif

#if defined(OS_MACOSX) || defined(OS_LINUX) || defined(OS_ANDROID)
#define OS_POSIX 1
#endif

} // namespace rfl

#endif /* __LIBRFL_TYPES_H__ */
