if (STANDALONE_BUILD)
  find_package (LLVM REQUIRED COMPONENTS "support")
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
  llvm_map_components_to_libnames(LLVM_LIBRARIES Support )
endif ()

set(proto_dir ${LIBRFL_SRC_DIR}/rfl/proto)
add_custom_command(
  OUTPUT reflected.pb.cc reflected.pb.h
  COMMAND protoc -I ${proto_dir} ${proto_dir}/reflected.proto --cpp_out=${CMAKE_CURRENT_BINARY_DIR} --python_out=${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS protoc ${proto_dir}/reflected.proto
)

set (CMAKE_CXX_FLAGS "-fno-rtti -fno-exceptions ${LLVM_CXX_FLAGS} -Wno-strict-aliasing")
set (rfl_TARGET_TYPE SHARED)
set (rfl_PUBLIC_HEADERS
  annotations.h
  generator.h
  generator_util.h
  native_library.h
  reflected.h
  rfl_export.h
  types.h
  )
set (rfl_SOURCES
  ${rfl_PUBLIC_HEADERS}
  generator.cc
  native_library.cc
  reflected.cc
  reflected.pb.h
  reflected.pb.cc
  )
set (rfl_DEFINES RFL_IMPLEMENTATION)
set (rfl_DEPS protobuf_full)
set (rfl_PRIV_DEPS ${LLVM_LIBRARIES})

if (OS_LINUX)
  set (rfl_LIBS dl)
endif ()

add_module (rfl)

if (BUILD_TESTS)
  add_subdirectory (test)
endif ()

install (FILES
    ${rfl_PUBLIC_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/reflected.pb.h
  DESTINATION include/rfl)
install (FILES proto/reflected.proto DESTINATION include/rfl/proto)
install (TARGETS rfl LIBRARY DESTINATION lib)
