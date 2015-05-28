set (test_annotations_TARGET_TYPE SHARED)
set (test_annotations_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/package_manifest.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/package_manifest.h
  ${CMAKE_CURRENT_SOURCE_DIR}/object.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/object.h
  ${CMAKE_CURRENT_SOURCE_DIR}/test_annotations.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/test_annotations.h
  )
set (test_annotations_DEPS rfl)
add_module(test_annotations)

set (test_dependencies_TARGET_TYPE SHARED)
set (test_dependencies_DEPS test_annotations)
set (test_dependencies_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/test_dependecies.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/test_dependecies.h
  )
add_module(test_dependencies)

set (test_generator_TARGET_TYPE SHARED)
set (test_generator_SOURCES
  test_generator.cc
  )
set (test_generator_DEPS rfl test_annotations)
add_module(test_generator)

#get_property(generator_path TARGET test_generator PROPERTY LOCATION)

macro (add_rfl_module mid version)
  set (${mid}_rfl_SOURCES
    ${mid}_rfl.h
    ${mid}_rfl.cc
    ${mid}.ini
    )
  set (${mid}_rfl_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BUILD_DIR})
  set (${mid}_rfl_TARGET_TYPE SHARED)
  set (${mid}_rfl_DEPS ${mid})
  set (${mid}_rfl_RFL_IMPORTS)
  foreach (dep ${ARGN})
    list (APPEND ${mid}_rfl_DEPS "${dep}_rfl")
    list (APPEND ${mid}_rfl_RFL_IMPORTS "-i${dep}")
  endforeach ()
  foreach (dep ${${mid}_DEPS})
    list (APPEND ${mid}_rfl_RFL_IMPORTS "-l${dep}")
  endforeacH ()

  add_custom_command (OUTPUT ${${mid}_rfl_SOURCES}
    COMMAND rfl-scan -p ${CMAKE_BINARY_DIR}/compile_commands.json
      -basedir ${CMAKE_SOURCE_DIR}
      -output ${mid}
      -pkg-name ${mid} -pkg-version=${version}
      ${${mid}_rfl_RFL_IMPORTS}
      -G $<TARGET_FILE:test_generator>
      ${${mid}_SOURCES}
    DEPENDS ${${mid}_SOURCES} ${${mid}_rfl_DEPS} rfl-scan test_generator
    )

  add_module (${mid}_rfl)
  set_module_output_directories(${mid}_rfl bin lib bin/plugins)
  add_custom_command(TARGET ${mid}_rfl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${mid}.ini
    ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/bin/plugins)
  set_target_properties(${mid}_rfl PROPERTIES
    INSTALL_RPATH "$ORIGIN/../../lib:$ORIGIN"
    BUILD_WITH_INSTALL_RPATH ON
    )
endmacro ()

add_rfl_module (test_annotations 1.0)
add_rfl_module (test_dependencies 1.0 test_annotations)

set (dump_class_TARGET_TYPE executable)
set (dump_class_SOURCES
  main.cc
  )
set (dump_class_DEPS test_annotations rfl)
add_module(dump_class)
