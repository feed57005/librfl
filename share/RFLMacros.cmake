# Adds new module with reflected sources.
# Reflected files are passed with <mid>_RFL_SOURCES variable
macro (add_module_rfl mid version)
  set (working_dir ${CMAKE_CURRENT_BINARY_DIR}/${mid})
  file(MAKE_DIRECTORY ${working_dir})

  set (input_files)
  set (output_files
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}/${mid}_export.rfl.h
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}/${mid}.rfl.h
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}/${mid}.rfl.cc
    )

  # used to cut path to be relative to CMAKE_SOURCE_DIR
  string(LENGTH ${CMAKE_SOURCE_DIR} src_dir_len)

  # prepare list of input & output files (TODO this actually depends on generator)
  foreach (src ${${mid}_RFL_SOURCES})
    set (abs_src ${CMAKE_CURRENT_SOURCE_DIR}/${src})
    # get relative path to source directory
    string(SUBSTRING ${abs_src} ${src_dir_len} -1 rel_src)
    list (APPEND output_files
      ${CMAKE_CURRENT_BINARY_DIR}/${mid}/${rel_src}.rfl.cc
      ${CMAKE_CURRENT_BINARY_DIR}/${mid}/${rel_src}.rfl.h
      )
    list (APPEND input_files ${abs_src})
  endforeach ()

  set (rfl_args)
  foreach (dep ${${mid}_DEPS})
    list (APPEND rfl_args "-l${dep}")
  endforeach ()
  foreach (imp ${${mid}_RFL_IMPORTS})
    list (APPEND rfl_args "-i${imp}")
    list (APPEND ${mid}_INCLUDE_DIRS ${${imp}_RFL_INCLUDE_DIR})
  endforeach ()

  add_custom_command (
    DEPENDS ${input_files} ${RFL_GENERATOR}
    COMMAND ${LIBRFL_RFLSCAN_EXE} -p ${CMAKE_BINARY_DIR}
      -basedir ${CMAKE_SOURCE_DIR}
      -output-dir ${mid}
      -pkg-name ${mid} -pkg-version=${version}
      ${imports}
      -G $<TARGET_FILE:${RFL_GENERATOR}>
      -verbose=${RFL_VERBOSE}
      ${input_files}
    WORKING_DIRECTORY ${working_dir}
    COMMENT Generating ${mid} to ${working_dir}
    OUTPUT ${output_files}
    )

  # setup <mid>_rfl module
  set (${mid}_rfl_SOURCES ${output_files})
  set (${mid}_rfl_TARGET_TYPE STATIC)
  set (${mid}_rfl_DEPS ${${mid}_DEPS})
  set (${mid}_rfl_INCLUDE_DIRS ${working_dir})

  add_module (${mid}_rfl)

  set (${mid}_RFL_INCLUDE_DIR ${working_dir} CACHE INTERNAL "" FORCE)

  # add to include dirs so that sources can use reflected headers
  # and rfl module as dependcy
  list (APPEND ${mid}_INCLUDE_DIRS ${working_dir})
  list (APPEND ${mid}_DEPS ${mid}_rfl)

  add_module (${mid})

endmacro ()

macro (add_rfl_sources mid version)
  set (${mid}_rfl_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}.rfl.h
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}.rfl.cc
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}_export.rfl.h
    )
  set (${mid}_rfl_DEPS ${mid} ${${mid}_DEPS})
  set (${mid}_rfl_RFL_IMPORTS)
  foreach (dep ${${mid}_DEPS})
    list (APPEND ${mid}_rfl_RFL_IMPORTS "-l${dep}")
  endforeach ()
  set (input_files)
  foreach (src ${ARGN})
    list (APPEND input_files ${CMAKE_CURRENT_SOURCE_DIR}/${src})
    list (APPEND ${mid}_rfl_SOURCES
      ${CMAKE_CURRENT_BINARY_DIR}/${src}.rfl.cc
      ${CMAKE_CURRENT_BINARY_DIR}/${src}.rfl.h
      )
  endforeach ()
  add_custom_command (
    DEPENDS ${input_files} ${RFL_GENERATOR}
    OUTPUT ${${mid}_rfl_SOURCES}
    COMMAND ${LIBRFL_RFLSCAN_EXE} -p ${CMAKE_BINARY_DIR}
      -basedir ${CMAKE_SOURCE_DIR}
      -output ${mid}
      -pkg-name ${mid} -pkg-version=${version}
      ${${mid}_rfl_RFL_IMPORTS}
      -G $<TARGET_FILE:${RFL_GENERATOR}>
      ${input_files}
    )
  set_source_files_properties (${${mid}_rfl_SOURCES} PROPERTIES GENERATED TRUE )
  list (APPEND ${mid}_SOURCES ${${mid}_rfl_SOURCES} ${ARGN})
endmacro ()

macro (add_rfl_module mid version)
  set (${mid}_rfl_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}.rfl.h
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}.rfl.cc
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}_export.rfl.h
    )
  set (${mid}_rfl_INCLUDE_DIRS ${${mid}_INCLUDE_DIRS} ${CMAKE_CURRENT_BUILD_DIR})
  set (${mid}_rfl_TARGET_TYPE SHARED)
  set (${mid}_rfl_DEPS ${mid} ${${mid}_DEPS})
  set (${mid}_rfl_RFL_IMPORTS)
  foreach (dep ${ARGN})
    list (APPEND ${mid}_rfl_DEPS "${dep}_rfl")
    list (APPEND ${mid}_rfl_RFL_IMPORTS "-i${dep}")
  endforeach ()
  foreach (dep ${${mid}_DEPS})
    list (APPEND ${mid}_rfl_RFL_IMPORTS "-l${dep}")
  endforeach ()
  set (input_files)
  foreach (src ${${mid}_SOURCES})
    list (APPEND input_files ${CMAKE_CURRENT_SOURCE_DIR}/${src})
    list (APPEND ${mid}_rfl_SOURCES
      ${CMAKE_CURRENT_BINARY_DIR}/${src}.rfl.cc
      ${CMAKE_CURRENT_BINARY_DIR}/${src}.rfl.h
      )
  endforeach ()
  add_custom_command (OUTPUT ${${mid}_rfl_SOURCES}
    COMMAND ${LIBRFL_RFLSCAN_EXE} -p ${CMAKE_BINARY_DIR}
      -basedir ${CMAKE_SOURCE_DIR}
      -output ${mid}
      -pkg-name ${mid} -pkg-version=${version}
      ${${mid}_rfl_RFL_IMPORTS}
      -G $<TARGET_FILE:${RFL_GENERATOR}>
      -plugin
      ${input_files}
    DEPENDS ${${mid}_SOURCES} ${${mid}_rfl_DEPS} ${RFL_GENERATOR}
    )

  add_module (${mid}_rfl)
  set_module_output_directories(${mid}_rfl bin lib bin/plugins)
  add_custom_command(TARGET ${mid}_rfl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${mid}.ini
    ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/bin/plugins)

  if (OS_LINUX)
    set (rpath_value "$ORIGIN")
  elseif (OS_MAC)
    set (rpath_value "$ORIGIN")
    #set (rpath_value "@executable_path")
  endif ()

  set_target_properties(${mid}_rfl PROPERTIES
    INSTALL_RPATH "${rpath_value}/../../lib:${rpath_value}"
    BUILD_WITH_INSTALL_RPATH ON
    )
endmacro ()
