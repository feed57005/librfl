set (gtest_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest)

set (gtest_INCLUDE_DIRS ${gtest_SOURCE_DIR} ${gtest_SOURCE_DIR}/include)
set (gtest_SOURCES
  ${gtest_SOURCE_DIR}/src/gtest-all.cc
  )

if (OS_POSIX)
  list (APPEND gtest_DEFINES "GTEST_HAS_RTTI=0")
  list (APPEND gtest_DEPS pthread)
endif ()

if (OS_ANDROID)
  list (APPEND gtest_DEFINES "GTEST_USE_OWN_TR1_TUPLE=1;GTEST_HAS_TR1_TUPLE=1")
endif ()

if (OS_WIN)
  list (APPEND gtest_DEFINES "_VARIADIC_MAX=10")
endif ()

set (gtest_PUBLIC_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include CACHE INTERNAL "" FORCE)
set (gtest_PUBLIC_DEFINES "UNIT_TEST;${gtest_DEFINES}" CACHE INTERNAL "" FORCE)

set (gtest_main_INCLUDE_DIRS ${gtest_INCLUDES})
set (gtest_main_SOURCES
  ${gtest_SOURCE_DIR}/src/gtest_main.cc
  )
set (gtest_main_DEPS gtest)
