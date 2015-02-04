cmake_minimum_required (VERSION 2.8)
include(umake/UMakeInit.cmake)

if (COMPILER_CLANG OR COMPILER_GCC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -std=c++11")
endif ()

add_subdirectory (third_party/cmph)

include_directories(. ${CMAKE_BINARY_DIR})
add_subdirectory (rfl)
add_subdirectory (rfl-scan)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
add_executable(python_wrap sandbox/python_wrap.cc)
target_link_libraries (python_wrap ${PYTHON_LIBRARIES})
