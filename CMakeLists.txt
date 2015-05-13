cmake_minimum_required (VERSION 2.8)
include(umake/UMakeInit.cmake)

option (BUILD_TESTS "Build tests" ON)

if (BUILD_TESTS)
	add_subdirectory (third_party/test)
endif ()

if (COMPILER_CLANG OR COMPILER_GCC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -std=c++11")
endif ()

include_directories(. ${CMAKE_BINARY_DIR})
add_subdirectory (rfl)
add_subdirectory (rfl-scan)

if (BUILD_TESTS)
	add_subdirectory(example)
endif ()

include (umake/UMakePackaging.cmake)

install (FILES share/FindLibRFL.cmake share/RFLMacros.cmake DESTINATION share/librfl)
