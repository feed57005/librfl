macro (add_rfl_sources mid version)
  set (${mid}_rfl_SOURCES
    ${mid}_rfl.h
    ${mid}_rfl.cc
    ${mid}.ini
    )
  set (${mid}_rfl_INCLUDE_DIRS ${${mid}_INCLUDE_DIRS} ${CMAKE_CURRENT_BUILD_DIR})
  set (${mid}_rfl_TARGET_TYPE SHARED)
  set (${mid}_rfl_DEPS ${mid} ${${mid}_DEPS})
  set (${mid}_rfl_RFL_IMPORTS)
  foreach (dep ${ARGN})
    list (APPEND ${mid}_rfl_RFL_IMPORTS "-i${dep}")
  endforeach ()
  foreach (dep ${${mid}_DEPS})
    list (APPEND ${mid}_rfl_RFL_IMPORTS "-l${dep}")
  endforeach ()
  set (annotated_sources)
  foreach (src ${${mid}_SOURCES})
    list (APPEND annotated_sources ${CMAKE_CURRENT_SOURCE_DIR}/${src})
  endforeach ()
  add_custom_command (OUTPUT ${${mid}_rfl_SOURCES}
    COMMAND ${LIBRFL_RFLSCAN_EXE} -p ${CMAKE_BINARY_DIR}/compile_commands.json
      -basedir ${CMAKE_SOURCE_DIR}
      -output ${mid}
      -pkg-name ${mid} -pkg-version=${version}
      ${${mid}_rfl_RFL_IMPORTS}
      -G $<TARGET_FILE:${RFL_GENERATOR}>
      ${annotated_sources}
    DEPENDS ${${mid}_SOURCES} ${RFL_GENERATOR}
    )
  list (APPEND ${mid}_SOURCES ${${mid}_rfl_SOURCES})
endmacro ()

macro (add_rfl_module mid version)
  set (${mid}_rfl_SOURCES
    ${mid}_rfl.h
    ${mid}_rfl.cc
    ${mid}.ini
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
  endforeach ()
  add_custom_command (OUTPUT ${${mid}_rfl_SOURCES}
    COMMAND ${LIBRFL_RFLSCAN_EXE} -p ${CMAKE_BINARY_DIR}/compile_commands.json
      -basedir ${CMAKE_SOURCE_DIR}
      -output ${mid}
      -pkg-name ${mid} -pkg-version=${version}
      ${${mid}_rfl_RFL_IMPORTS}
      -G $<TARGET_FILE:${RFL_GENERATOR}>
      ${annotated_sources}
    DEPENDS ${${mid}_SOURCES} ${${mid}_rfl_DEPS} ${RFL_GENERATOR}
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
