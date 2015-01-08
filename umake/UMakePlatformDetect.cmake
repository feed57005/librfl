# init os, arch, compiler vars
set (OS)
option (OS_WIN "" OFF)
option (OS_MAC "" OFF)
option (OS_LINUX "" OFF)
option (OS_ANDROID "" OFF)
option (OS_IOS "" OFF)
option (OS_NACL "" OFF)
option (OS_POSIX "" OFF)

set (ARCH_TYPE)
set (ARCH_TYPE_X86 OFF)
set (ARCH_TYPE_ARM OFF)
set (ARCH_TYPE_ARM_VER_7)

set (COMPILER)
set (COMPILER_GCC OFF)
set (COMPILER_MSVC OFF)
set (COMPILER_CLANG OFF)

# osx
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (OS_MAC ON)
  set (OS "MAC")
  set (OS_POSIX ON)
  set (ARCH "X86")
  set (ARCH_TYPE_X86 ON)
  set (COMPILER_CLANG ON)
  set (COMPILER "CLANG")
  set (MAC_DEFINES "_STDC_CONSTANT_MACROS;_STDC_FORMAT_MACROS")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (OS_LINUX ON)
  set (OS "LINUX")
  set (OS_POSIX ON)
  set (LINUX_DEFINES "__STDC_CONSTANT_MACROS;__STDC_FORMAT_MACROS")
  set (COMPILER "GCC")
  set (COMPILER_GCC ON)
  if (RPI_TOOLCHAIN)
    set (ARCH "ARM")
    set (ARCH_TYPE_ARM ON)
  else ()
    set (ARCH "X86")
    set (ARCH_TYPE_X86 ON)
  endif ()
  if (MODULE_TARGET_TYPE STREQUAL "SHARED")
    set (CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} -fPIC ")
    set (CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -fPIC ")
  endif ()
else ()
# windows
  set (OS_WIN ON)
  set (OS "WIN")
  # TODO cleanup those
  set (WIN_DEFINES
    CERT_CHAIN_PARA_HAS_EXTRA_FIELDS
    NOMINMAX
    PSAPI_VERSION=1
    WIN32
    WIN32_LEAN_AND_MEAN
    WINVER=0x0602
    _ATL_NO_OPENGL 
    _CRT_RAND_S
    _WIN32_WINNT=0x0602
    _WINDOWS
    )
  set (MSVC_COMPILE_FLAGS " /MP ")
  set (ARCH "X86")
  set (ARCH_TYPE_X86 ON)
  set (COMPILER_MSVC ON)
  set (COMPILER "MSVC")
  if (MODULE_TARGET_TYPE STREQUAL "SHARED")
    list (APPEND WIN_DEFINES COMPONENT_BUILD)
  endif ()
endif ()
