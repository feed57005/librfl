set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CROSSCOMPILING FALSE )

set (CLANG_ROOT "/opt/clang")

set (CMAKE_CXX_COMPILER ${CLANG_ROOT}/bin/clang++)
set (CMAKE_C_COMPILER ${CLANG_ROOT}/bin/clang)
set (CMAKE_AR ${CLANG_ROOT}/bin/llvm-ar)

set(LLVM_HOST)
set(CMAKE_FIND_ROOT_PATH "${CLANG_ROOT};${CMAKE_FIND_ROOT_PATH}")
include_directories(SYSTEM ${CLANG_ROOT}/include/c++/v1)

