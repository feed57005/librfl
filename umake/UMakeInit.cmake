# setup find_* paths
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/find_modules)

# export cc flags for code tools
set (CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

# default to Debug build type
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug CACHE INTERNAL "" FORCE)
endif ()

# setup OS & ARCH variables
include (${CMAKE_CURRENT_LIST_DIR}/UMakePlatformDetect.cmake)

# load macros
include (${CMAKE_CURRENT_LIST_DIR}/UMakeMacros.cmake)

# setup project
include (project_info.cmake)

if (NOT PROJECT_NAME)
  message (FATAL_ERROR "PROJECT_NAME not set in project_info.cmake")
endif ()

project (${PROJECT_NAME})
