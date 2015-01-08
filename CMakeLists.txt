cmake_minimum_required (VERSION 2.8)
include(umake/UMakeInit.cmake)

if (COMPILER_CLANG OR COMPILER_GCC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -std=c++11")
endif ()

add_subdirectory (third_party/cmph)

include_directories(. ${CMAKE_BINARY_DIR})
add_subdirectory (rfl)
add_subdirectory (rfl-scan)
