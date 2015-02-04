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

set (implicit_includes "${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")
if (OS_LINUX)
  list (APPEND implicit_includes
    /usr/include/x86_64-linux-gnu
    /usr/lib/gcc/x86_64-linux-gnu/4.7/include
    /usr/lib/gcc/x86_64-linux-gnu/4.7/include-fixed
    )
endif ()

foreach (impl ${implicit_includes})
  set (implicit "${implicit} -I${impl}")
endforeach ()
message ("xXX ${implicit}")
set (rfl-scan_DEFINES "IMPLICIT=\"${implicit}\"")
#set (implicit "")
#
set (CMAKE_CXX_FLAGS "-fno-rtti -fno-exceptions ${LLVM_CXX_FLAGS} -Wno-strict-aliasing")
#set (CMAKE_LD_FLAGS ${LLVM_LIBRARIES})
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

set (test_annotations_TARGET_TYPE SHARED)
set (test_annotations_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_annotations.cc
  )
add_module(test_annotations)

set (test_annotations_rfl_TARGET_TYPE SHARED)
set (test_annotations_rfl_SOURCES
  test_annotations_rfl.cc
  )

set (test_annotations_rfl_DEPS test_annotations rfl)

add_custom_command (OUTPUT test_annotations_rfl.cc
  COMMAND rfl-scan -p ${CMAKE_BINARY_DIR}/compile_commands.json -output test_annotations_rfl
  ${test_annotations_SOURCES}
  DEPENDS ${test_annotations_SOURCES} rfl-scan
  )

add_module (test_annotations_rfl)
