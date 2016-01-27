if (STANDALONE_BUILD)
  find_package (LLVM REQUIRED COMPONENTS "support" "asmparser" "bitreader" "mc" "mcparser" "option")

  include_directories (${LLVM_INCLUDE_DIRS})
  link_directories (${LLVM_LIBRARY_DIRS})
else ()
  include_directories("${CMAKE_SOURCE_DIR}/include"
                      "${CMAKE_SOURCE_DIR}/tools/clang/include"
                      "${CMAKE_BINARY_DIR}/include"
                      "${CMAKE_BINARY_DIR}/tools/clang/include")

  link_directories("${CMAKE_SOURCE_DIR}/lib"
                   "${CMAKE_SOURCE_DIR}/tools/clang/lib"
                   "${CMAKE_BINARY_DIR}/lib"
                   "${CMAKE_BINARY_DIR}/tools/clang/lib")
  llvm_map_components_to_libnames(LLVM_LIBRARIES Support BitReader MCParser Option X86AsmParser)
endif ()

set (rfl-scan_SOURCES
  annotation_parser.cc
  annotation_parser.h
  ast_scan.cc
  ast_scan.h
  compilation_db.cc
  compilation_db.h
  main.cc
  proto_ast_scan.cc
  proto_ast_scan.h
  )
set (rfl-scan_TARGET_TYPE executable)
set (implicit "")
set (implicit_includes)
set (implicit)

foreach (impl ${implicit_includes})
  set (implicit "${implicit} -I${impl}")
endforeach ()
set (rfl-scan_DEFINES "IMPLICIT=\"${implicit}\"")

set (CMAKE_CXX_FLAGS "-fno-rtti -fno-exceptions ${LLVM_CXX_FLAGS} -Wno-strict-aliasing")

set (rfl-scan_DEPS rfl protobuf_lite)
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
