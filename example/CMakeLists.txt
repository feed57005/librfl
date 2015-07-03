include (${CMAKE_SOURCE_DIR}/share/RFLMacros.cmake)
set (LIBRFL_RFLSCAN_EXE rfl-scan)

set (test_generator_TARGET_TYPE SHARED)
set (test_generator_SOURCES
  test_generator.cc
  )
set (test_generator_DEPS rfl)
add_module(test_generator)
set (RFL_GENERATOR test_generator)

####

set (test_annotations_TARGET_TYPE SHARED)
set (test_annotations_SOURCES
  package_manifest.cc
  package_manifest.h
  object.cc
  object.h
  test_annotations.cc
  test_annotations.h
  test_primitives.h
  )
set (test_annotations_DEPS rfl)
set (test_annotations_RFL_SOURCES
  test_annotations.h
  )
add_module_rfl(test_annotations 1.0)

####

set (test_dependencies_TARGET_TYPE SHARED)
set (test_dependencies_DEPS test_annotations)
set (test_dependencies_SOURCES
  test_dependecies.cc
  test_dependecies.h
  )
set (test_dependencies_RFL_SOURCES
  test_dependecies.h
  )
set (test_dependecies_RFL_IMPORTS test_annotations)
add_module_rfl(test_dependencies 1.0)

####

set (dump_class_TARGET_TYPE executable)
set (dump_class_SOURCES
  main.cc
  )
set (dump_class_DEPS test_annotations rfl)
add_module(dump_class)
