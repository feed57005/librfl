macro (set_module_version mid major minor api id )
  set (${mid}_MAJOR_VERSION ${major})
  set (${mid}_MINOR_VERSION ${minor})
  set (${mid}_ID_VERSION ${id})
  set (${mid}_API_VERSION ${api})
  set (${mid}_VERSION "${major}.${minor}-${id}")
  set (${mid}_PACKAGE_NAME "${mid}-${major}.${minor}-${id}")
  set (MODULE ${mid})
  string (TOUPPER ${mid} MODULE_UPPER)
endmacro ()

macro (add_module_version mid)
  set (templates_dir "${CMAKE_SOURCE_DIR}/build/umake/resources")
  configure_file (${templates_dir}/version.h.in ${mid}_version.h)
  list (APPEND ${mid}_SOURCES
    ${mid}_version.h
    )
endmacro ()


