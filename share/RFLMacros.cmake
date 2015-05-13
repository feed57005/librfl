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
