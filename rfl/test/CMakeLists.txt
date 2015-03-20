set (testpkg_SOURCES testpkg.cc)
set (testpkg_TARGET_TYPE executable)
set (testpkg_DEPS rfl)
#add_module (testpkg)

set (rfl_unittests_SOURCES
  ../reflected_unittest.cc
  ../dictionary_unittest.cc
  )
set (rfl_unittests_TARGET_TYPE unittest)
set (rfl_unittests_DEPS rfl gtest gtest_main)
add_module (rfl_unittests)
