# Find Clang
#
# It defines the following variables
# CLANG_FOUND        - True if Clang found.
# CLANG_INCLUDE_DIRS - where to find Clang include files
# CLANG_LIBS         - list of clang libs

if (NOT LLVM_INCLUDE_DIRS OR NOT LLVM_LIBRARY_DIRS)
  message(FATAL_ERROR "No LLVM and Clang support requires LLVM")
else (NOT LLVM_INCLUDE_DIRS OR NOT LLVM_LIBRARY_DIRS)

macro(find_and_add_clang_lib _libname_)
  find_library(CLANG_${_libname_}_LIB ${_libname_} ${LLVM_LIBRARY_DIRS} ${CLANG_LIBRARY_DIRS})
  if (CLANG_${_libname_}_LIB)
    set(CLANG_LIBS ${CLANG_LIBS} ${CLANG_${_libname_}_LIB})
  endif ()
endmacro()

# Clang shared library provides just the limited C interface, so it
# can not be used.  We look for the static libraries.
find_and_add_clang_lib (clangFrontend)
find_and_add_clang_lib (clangDriver)
find_and_add_clang_lib (clangCodeGen)
find_and_add_clang_lib (clangEdit)
find_and_add_clang_lib (clangSema)
find_and_add_clang_lib (clangChecker)
find_and_add_clang_lib (clangAnalysis)
find_and_add_clang_lib (clangRewrite)
find_and_add_clang_lib (clangAST)
find_and_add_clang_lib (clangParse)
find_and_add_clang_lib (clangLex)
find_and_add_clang_lib (clangBasic)

find_path (CLANG_INCLUDE_DIRS clang/Basic/Version.h HINTS ${LLVM_INCLUDE_DIRS})

if (CLANG_LIBS AND CLANG_INCLUDE_DIRS)
  message(STATUS "Clang libs: " ${CLANG_LIBS})
  set(CLANG_FOUND TRUE)
endif (CLANG_LIBS AND CLANG_INCLUDE_DIRS)

if (CLANG_FOUND)
  message(STATUS "Found Clang: ${CLANG_INCLUDE_DIRS}")
else (CLANG_FOUND)
  if (CLANG_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Clang")
  endif (CLANG_FIND_REQUIRED)
endif (CLANG_FOUND)

endif (NOT LLVM_INCLUDE_DIRS OR NOT LLVM_LIBRARY_DIRS)
