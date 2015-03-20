set (test_generator_TARGET_TYPE SHARED)
set (test_generator_SOURCES
  test_generator.cc
  )
set (test_generator_DEPS rfl)
add_module(test_generator)

get_property(generator_path TARGET test_generator PROPERTY LOCATION)

#
set (test_annotations_TARGET_TYPE SHARED)
set (test_annotations_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/object.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/object.h
  ${CMAKE_CURRENT_SOURCE_DIR}/test_annotations.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/test_annotations.h
  )
add_module(test_annotations)

set (test_dependencies_TARGET_TYPE SHARED)
set (test_dependencies_DEPS test_annotations)
set (test_dependencies_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/test_dependecies.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/test_dependecies.h
  )
add_module(test_dependencies)

macro (add_rfl_module mid version)
  set (${mid}_rfl_SOURCES
    ${mid}_rfl.cc
    )
  set (${mid}_rfl_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BUILD_DIR})
  set (${mid}_rfl_TARGET_TYPE SHARED)
  set (${mid}_rfl_DEPS ${mid} rfl)

  add_custom_command (OUTPUT ${${mid}_rfl_SOURCES}
    COMMAND rfl-scan -p ${CMAKE_BINARY_DIR}/compile_commands.json
      -basedir ${CMAKE_SOURCE_DIR}
      -output ${mid}_rfl
      -pkg-name ${mid} -pkg-version=${version}
      -G ${generator_path}
      ${${mid}_SOURCES}
    DEPENDS ${${mid}_SOURCES} rfl-scan test_generator
    )
  add_module (${mid}_rfl)
endmacro ()

add_rfl_module (test_annotations 1.0)
add_rfl_module (test_dependencies 1.0)

set (dump_class_TARGET_TYPE executable)
set (dump_class_SOURCES
  main.cc
  )
set (dump_class_DEPS test_annotations rfl)
add_module(dump_class)

#set (test_annotations_rfl_TARGET_TYPE SHARED)
#set (test_annotations_rfl_SOURCES
#  test_annotations_rfl.cc
#  )
#
#set (test_annotations_rfl_DEPS test_annotations rfl)
#
#add_custom_command (OUTPUT test_annotations_rfl.cc
#  COMMAND rfl-scan -p ${CMAKE_BINARY_DIR}/compile_commands.json
#    -basedir ${CMAKE_SOURCE_DIR}
#    -output test_annotations_rfl
#    -pkg-name test_annotations -pkg-version=1.0
#    -G ${generator_path}
#    ${test_annotations_SOURCES}
#  DEPENDS ${test_annotations_SOURCES} rfl-scan test_generator
#  )
#
#add_module (test_annotations_rfl)
