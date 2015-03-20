set (rfl_TARGET_TYPE SHARED)
set (rfl_SOURCES
  annotations.h
  dictionary.cc
  dictionary.h
  generator.cc
  generator.h
  native_library.cc
  native_library.h
  reflected.cc
  reflected.h
  reflected_io.cc
  reflected_io.h
  repository.cc
  repository.h
  rfl_export.h
  types.h
  )

if (OS_LINUX)
  set (rfl_LIBS dl)
endif ()

add_module (rfl)

add_subdirectory (test)
