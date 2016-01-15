cmake_minimum_required (VERSION 2.8)
include (umake/UMakeInit.cmake)
include (umake/UMakePackaging.cmake)

option (BUILD_TESTS "Build tests" OFF)

add_subdirectory (third_party/protobuf)
if (BUILD_TESTS)
  add_subdirectory (third_party/test)
endif ()

if (COMPILER_CLANG OR COMPILER_GCC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -std=c++11")
endif ()

include_directories (. ${CMAKE_BINARY_DIR})

add_subdirectory (rfl)
add_subdirectory (rfl-scan)
add_subdirectory (rfl-gen)

if (BUILD_TESTS)
  add_subdirectory(example)
endif ()

install (FILES
    share/FindLibRFL.cmake
    share/RFLMacros.cmake
    share/UseRFL.cmake
  DESTINATION share/librfl)
