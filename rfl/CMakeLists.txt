set (rfl_TARGET_TYPE SHARED)
set (rfl_PUBLIC_HEADERS
  annotations.h
  dictionary.h
  generator.h
  native_library.h
  reflected.h
  repository.h
  rfl_export.h
  types.h
	)
set (rfl_SOURCES
	${rfl_PUBLIC_HEADERS}
  dictionary.cc
  generator.cc
  native_library.cc
  reflected.cc
  repository.cc
  )

if (OS_LINUX)
  set (rfl_LIBS dl)
endif ()

add_module (rfl)

if (BUILD_TESTS)
	add_subdirectory (test)
endif ()
install (FILES ${rfl_PUBLIC_HEADERS} DESTINATION include)
install (TARGETS rfl LIBRARY DESTINATION lib)
