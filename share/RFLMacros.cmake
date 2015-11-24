macro (rfl_scan mid version rfl_files_var)
  set (rfl_args)
  foreach (dep ${${mid}_DEPS})
    list (APPEND rfl_args "-l${dep}")
  endforeach ()

  foreach (imp ${${mid}_RFL_IMPORTS})
    list (APPEND rfl_args "-i${imp}")
  endforeach ()

  set (scans)
  set (rfl_files)
  set (working_dir ${CMAKE_CURRENT_BINARY_DIR})
  foreach (src ${${mid}_RFL_SOURCES})
    set (input_file ${CMAKE_CURRENT_SOURCE_DIR}/${src})
    add_custom_target(${mid}_scan_${src}
      COMMAND ${LIBRFL_RFLSCAN_EXE}
        -p ${CMAKE_BINARY_DIR}
        -basedir ${CMAKE_SOURCE_DIR}
        -output-dir ${working_dir}/${mid}
        -pkg-name ${mid} -pkg-version=${version}
        -verbose=${RFL_VERBOSE}
        -proto
        -o ${src}.rfl
        ${input_file}
      WORKING_DIRECTORY ${working_dir}
      COMMENT Scanning ${src} to ${working_dir}
      DEPENDS ${src} ${LIBRFL_RFLSCAN_EXE}
      BYPRODUCTS ${src}.rfl
      )
    list(APPEND scans ${mid}_scan_${src})
    list(APPEND rfl_files ${src}.rfl)
  endforeach ()

  add_custom_target(${mid}_rfl_scan DEPENDS ${scans})
  set(${rfl_files_var} ${rfl_files} PARENT_SCOPE)
endmacro ()

macro (rfl_gen mid version)
  unset(rfl_files)
  rfl_scan(${mid} ${version} rfl_files)

  # used to cut path to be relative to CMAKE_SOURCE_DIR
  string(LENGTH "${CMAKE_SOURCE_DIR}/" src_dir_len)
  set (input_args "-i")
  foreach (src ${${mid}_RFL_SOURCES})
    set (input_file ${CMAKE_CURRENT_SOURCE_DIR}/${src})
    string(SUBSTRING ${input_file} ${src_dir_len} -1 input_file)
    list (APPEND input_args "${input_file}")
  endforeach ()

  set (plugin_arg)
  if (${mid}_RFL_PLUGIN)
    set (plugin_arg "--plugin")
  endif()

  set (working_dir ${CMAKE_CURRENT_BINARY_DIR})
  execute_process(COMMAND
    ${LIBRFL_RFLGEN_PY} -g ${RFL_RFLGEN_GENERATOR}
      -o ${working_dir}/${mid}
      ${input_args}
      ${plugin_arg}
      --pkg-name "${mid}"
      --pkg-version "${version}"
      --print-files
    WORKING_DIRECTORY ${working_dir}
    OUTPUT_VARIABLE out_files
    #RESULT_VARIABLE out_result
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  string (REPLACE "\n" ";" out_files "${out_files}")

  set (input_args "-i")
  foreach (rfl_file ${rfl_files})
    list(APPEND input_args "${rfl_file}")
  endforeach ()
  add_custom_command(
    OUTPUT ${out_files}
    COMMAND ${LIBRFL_RFLGEN_PY} -g ${RFL_RFLGEN_GENERATOR}
      -o ${mid}
      ${input_args}
      ${plugin_arg}
      --pkg-name "${mid}"
      --pkg-version "${version}"
    DEPENDS ${mid}_rfl_scan ${rfl_files}
    WORKING_DIRECTORY ${working_dir}
    COMMENT Generating ${mid} to ${working_dir}
    )

  # setup <mid>_rfl module
  set (${mid}_rfl_SOURCES ${out_files})
  set (${mid}_rfl_TARGET_TYPE STATIC)
  set (${mid}_rfl_DEPS ${${mid}_DEPS})
  set (${mid}_rfl_INCLUDE_DIRS ${working_dir}/${mid})
  set (${mid}_rfl_DEFINES ${${mid}_DEFINES})

  add_module (${mid}_rfl)

  set (${mid}_RFL_INCLUDE_DIR ${working_dir}/${mid} CACHE INTERNAL "" FORCE)

  # add to include dirs so that sources can use reflected headers
  # and rfl module as dependcy
  list (APPEND ${mid}_INCLUDE_DIRS ${working_dir}/${mid})
  list (APPEND ${mid}_DEPS ${mid}_rfl)
endmacro ()

# Adds new module with reflected sources.
# Reflected files are passed with <mid>_RFL_SOURCES variable
macro (add_module_rfl mid version)
  set (working_dir ${CMAKE_CURRENT_BINARY_DIR})
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
      -proto
      -o ${mid}.rfl
      ${input_files}
    WORKING_DIRECTORY ${working_dir}
    COMMENT Generating ${mid} to ${working_dir}
    OUTPUT ${output_files}
    )

  # setup <mid>_rfl module
  set (${mid}_rfl_SOURCES ${output_files})
  set (${mid}_rfl_TARGET_TYPE STATIC)
  set (${mid}_rfl_DEPS ${${mid}_DEPS})
  set (${mid}_rfl_INCLUDE_DIRS ${working_dir}/${mid})
  set (${mid}_rfl_DEFINES ${${mid}_DEFINES})

  add_module (${mid}_rfl)

  set (${mid}_RFL_INCLUDE_DIR ${working_dir}/${mid} CACHE INTERNAL "" FORCE)

  # add to include dirs so that sources can use reflected headers
  # and rfl module as dependcy
  list (APPEND ${mid}_INCLUDE_DIRS ${working_dir}/${mid})
  list (APPEND ${mid}_DEPS ${mid}_rfl)

  add_module (${mid})

  set_module_output_directories(${mid} bin lib bin/plugins/${mid})
  add_custom_command(TARGET ${mid} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${mid}/${mid}.ini
    ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/bin/plugins/${mid}/${mid}.ini)

  if (OS_LINUX)
    set (rpath_value "$ORIGIN")
  elseif (OS_MAC)
    set (rpath_value "$ORIGIN")
    #set (rpath_value "@executable_path")
  endif ()

  set_target_properties(${mid} PROPERTIES
    INSTALL_RPATH "${rpath_value}/../../lib:${rpath_value}"
    BUILD_WITH_INSTALL_RPATH ON
    )
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
