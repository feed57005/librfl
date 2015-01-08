set(CMAKE_CROSSCOMPILING FALSE )

set (CLANG_ROOT "/opt/clang-3.5")

set (CMAKE_CXX_COMPILER ${CLANG_ROOT}/bin/clang++)
set (CMAKE_C_COMPILER ${CLANG_ROOT}/bin/clang)

set(LLVM_HOST)
#set(CMAKE_FIND_ROOT_PATH  ${CLANG_ROOT})
set(osx_toolchain "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain")
#set(CMAKE_CXX_FLAGS "-isystem ${osx_toolchain}/usr/lib/c++/v1 -isystem ${osx_toolchain}/usr/include" CACHE STRING "" FORCE)
include_directories(SYSTEM ${osx_toolchain}/usr/lib/c++/v1 ${osx_toolchain}/usr/include)
