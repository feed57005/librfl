# setup rfl cmake macros
include (${CMAKE_SOURCE_DIR}/share/RFLMacros.cmake)
set (RFL_GENERATOR test_generator)
set (RFL_VERBOSE 0)
set (LIBRFL_RFLSCAN_EXE rfl-scan)

set (test_generator_TARGET_TYPE SHARED)
set (test_generator_SOURCES
  test_generator.cc
  )
set (test_generator_DEPS rfl)
add_module(test_generator)

####

set (example_TARGET_TYPE SHARED)
set (example_SOURCES
  any_var.h
  call_desc.h
  enum_class.h
  example_export.h
  object.cc
  object.h
  package_manifest.cc
  package_manifest.h
  property.cc
  property.h
  type_class.h
  type_info.h
  type_repository.cc
  type_repository.h
  )
set (example_DEFINES EXAMPLE_IMPLEMENTATION)
set (example_DEPS rfl)
add_module(example)

set (test_annotations_TARGET_TYPE SHARED)
set (test_annotations_SOURCES
  test_annotations.cc
  test_annotations.h
  test_primitives.h
  )
set (test_annotations_DEPS rfl example)
set (test_annotations_RFL_SOURCES
  test_annotations.h
  test_primitives.h
  )
set (test_annotations_DEFINES TEST_ANNOTATIONS_IMPLEMENTATION)
add_module_rfl(test_annotations 1.0)

####

set (test_dependencies_TARGET_TYPE SHARED)
set (test_dependencies_DEPS example)
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
