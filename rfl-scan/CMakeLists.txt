find_package (LLVM REQUIRED COMPONENTS "support" "asmparser" "bitreader" "mc" "mcparser" "option")

include_directories (${LLVM_INCLUDE_DIRS})
link_directories (${LLVM_LIBRARY_DIRS})

set (rfl-scan_SOURCES
  annotation_parser.cc
  annotation_parser.h
  ast_scan.cc
  ast_scan.h
  main.cc
  )
set (rfl-scan_TARGET_TYPE executable)
set (implicit "")

# FIXME grab implicit includes by running compiler
#   clang++ -E -x c++ - -v < /dev/null
#   g++ -E -x c++ - -v < /dev/null
set (implicit_includes "${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")
if (OS_LINUX)
  list (APPEND implicit_includes
    /usr/include/x86_64-linux-gnu
    /usr/lib/gcc/x86_64-linux-gnu/4.7/include
    /usr/lib/gcc/x86_64-linux-gnu/4.7/include-fixed
    )
elseif (OS_MAC)
  set (toolchain_path /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain)
  list (APPEND implicit_includes
    ${toolchain_path}/usr/include/c++/v1
    ${toolchain_path}/usr/lib/clang/6.0/include/
    ${toolchain_path}/usr/include
    )
endif ()

foreach (impl ${implicit_includes})
  set (implicit "${implicit} -I${impl}")
endforeach ()
message ("xXX ${implicit}")
set (rfl-scan_DEFINES "IMPLICIT=\"${implicit}\"")

set (CMAKE_CXX_FLAGS "-fno-rtti -fno-exceptions ${LLVM_CXX_FLAGS} -Wno-strict-aliasing")

set (rfl-scan_DEPS rfl)
set (rfl-scan_LIBS
  clangFrontend
  clangSerialization
  clangDriver
  clangTooling
  clangParse
  clangSema
  clangStaticAnalyzerFrontend
  clangStaticAnalyzerCheckers
  clangStaticAnalyzerCore
  clangAnalysis
  clangRewriteFrontend
  clangRewrite
  clangEdit
  clangAST
  clangLex
  clangBasic
  ${LLVM_LIBRARIES}
  )
add_module (rfl-scan)
if (OS_MAC)
  set (rpath "@executable_path/../lib")
elseif (OS_LINUX)
  set (rpath "$ORIGIN/../lib")
endif ()
set_target_properties(rfl-scan PROPERTIES
	INSTALL_RPATH "${rpath}"
	BUILD_WITH_INSTALL_RPATH ON
	)

install (TARGETS rfl-scan RUNTIME DESTINATION bin)
