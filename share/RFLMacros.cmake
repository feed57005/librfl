macro (add_module_rfl mid version)
  set (output_files
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}_export.rfl.h
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}.rfl.h
    ${CMAKE_CURRENT_BINARY_DIR}/${mid}.rfl.cc
    )
  set (annotated_sources)
  foreach (src ${${mid}_RFL_SOURCES})
    list (APPEND annotated_sources ${CMAKE_CURRENT_SOURCE_DIR}/${src})
    list (APPEND output_files
      ${CMAKE_CURRENT_BINARY_DIR}/${src}.rfl.cc
      ${CMAKE_CURRENT_BINARY_DIR}/${src}.rfl.h
      )
  endforeach ()

  set (imports)
  foreach (dep ${${mid}_DEPS})
    list (APPEND imports "-l${dep}")
  endforeach ()

  add_custom_command (
    DEPENDS ${annotated_sources} ${RFL_GENERATOR}
    COMMAND ${LIBRFL_RFLSCAN_EXE} -p ${CMAKE_BINARY_DIR}
      -basedir ${CMAKE_SOURCE_DIR}
      -output ${mid}
      -pkg-name ${mid} -pkg-version=${version}
      ${imports}
      -G $<TARGET_FILE:${RFL_GENERATOR}>
      ${annotated_sources}
    OUTPUT ${output_files}
    )
  set (${mid}_rfl_SOURCES ${output_files})
  set (${mid}_rfl_TARGET_TYPE STATIC)
  set (${mid}_rfl_DEPS ${${mid}_DEPS})

  add_module(${mid}_rfl)

  list (APPEND ${mid}_DEPS ${mid}_rfl)
  list (APPEND ${mid}_SOURCES ${${mid}_RFL_SOURCES})

  add_module(${mid})

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
  set (annotated_sources)
  foreach (src ${ARGN})
    list (APPEND annotated_sources ${CMAKE_CURRENT_SOURCE_DIR}/${src})
    list (APPEND ${mid}_rfl_SOURCES
      ${CMAKE_CURRENT_BINARY_DIR}/${src}.rfl.cc
      ${CMAKE_CURRENT_BINARY_DIR}/${src}.rfl.h
      )
  endforeach ()
  add_custom_command (
    DEPENDS ${annotated_sources} ${RFL_GENERATOR}
    OUTPUT ${${mid}_rfl_SOURCES}
    COMMAND ${LIBRFL_RFLSCAN_EXE} -p ${CMAKE_BINARY_DIR}
      -basedir ${CMAKE_SOURCE_DIR}
      -output ${mid}
      -pkg-name ${mid} -pkg-version=${version}
      ${${mid}_rfl_RFL_IMPORTS}
      -G $<TARGET_FILE:${RFL_GENERATOR}>
      ${annotated_sources}
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
  set (annotated_sources)
  foreach (src ${${mid}_SOURCES})
    list (APPEND annotated_sources ${CMAKE_CURRENT_SOURCE_DIR}/${src})
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
      ${annotated_sources}
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
