cmake_minimum_required (VERSION 2.8)
include (umake/UMakeInit.cmake)

# If we are not building as a part of LLVM, build librfl as an
# standalone project, using LLVM as an external library:
set (STANDALONE_BUILD OFF)
set (LIBRFL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (LIBRFL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
message("${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR}")
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set (STANDALONE_BUILD ON)
  project (${PROJECT_NAME})
  include (umake/UMakePackaging.cmake)
endif ()

option (BUILD_TESTS "Build tests" OFF)

add_subdirectory (third_party/protobuf)
if (BUILD_TESTS)
  add_subdirectory (third_party/test)
endif ()

set (CLANG_COMPILE_FLAGS "-fPIC -fno-rtti -fno-exceptions -stdlib=libc++ -std=c++11")
set (GCC_COMPILE_FLAGS "${CLANG_COMPILE_FLAGS}")

include_directories (${LIBRFL_SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR})

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
    share/rfl_rule.gni
    share/rfl_tool.py
  DESTINATION share/librfl)
