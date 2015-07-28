#ifndef __EXAMPLE_EXPORT_RFL_H__
#define __EXAMPLE_EXPORT_RFL_H__

#if defined(WIN32)
#if defined(EXAMPLE_IMPLEMENTATION)
#define EXAMPLE_EXPORT __declspec(dllexport)
#else
#define EXAMPLE_EXPORT __declspec(dllimport)
#endif

#else // defined(WIN32)
#if defined(EXAMPLE_IMPLEMENTATION)
#define EXAMPLE_EXPORT __attribute__((visibility("default")))
#else
#define EXAMPLE_EXPORT
#endif
#endif

#endif // __EXAMPLE_EXPORT_RFL_H_
