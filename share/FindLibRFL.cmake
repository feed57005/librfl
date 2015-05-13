# Find librfl
# Defines the following variables:
# LIBRFL_RFLSCAN_EXE
# LIBRFL_FOUND
# LIBRFL_INCLUDE_DIRS
# LIBRFL_LIBRARIES

find_program(_rfl_scan_exe
  NAMES rfl-scan
  HINTS $ENV{LIBRFL_PATH}/bin
  DOC "rfl-scan executable location")

find_library(_librfl
  NAMES rfl librfl
  HINTS $ENV{LIBRFL_PATH}/lib
)

find_path(LIBRFL_INCLUDE_DIRS
  reflected.h
  HINTS $ENV{LIBRFL_PATH}/include
  )

if (_librfl)
  set (LIBRFL_LIBRARIES ${_librfl} CACHE INTERNAL "" FORCE)
endif ()

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBRFL
  DEFAULT_MSG
  LIBRFL_RFLSCAN_EXE
  LIBRFL_LIBRARIES
  LIBRFL_INCLUDE_DIRS)

if (LIBRFL_FOUND)
  set (LIBRFL_FOUND TRUE CACHE INTERNAL "")
endif ()
