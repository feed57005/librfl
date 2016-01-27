set(bindir ${CMAKE_BINARY_DIR}/bin)
set(libdir ${CMAKE_BINARY_DIR}/lib/rfl-gen)
set(protobuf_dir ${LIBRFL_SRC_DIR}/third_party/protobuf)
set(proto_dir ${protobuf_dir}/src/google/protobuf)

add_custom_target(rfl-gen ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${bindir}
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBRFL_SRC_DIR}/rfl-gen/rfl-gen ${bindir}
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBRFL_SRC_DIR}/rfl-gen/rfl-dump ${bindir}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${libdir}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIBRFL_SRC_DIR}/rfl-gen/rfl ${libdir}/rfl
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIBRFL_SRC_DIR}/rfl-gen/templates ${libdir}/templates
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIBRFL_SRC_DIR}/third_party/jinja2/jinja2 ${libdir}/jinja2
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIBRFL_SRC_DIR}/third_party/protobuf/python/google ${libdir}/google
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBRFL_BINARY_DIR}/rfl/reflected_pb2.py ${libdir}/rfl/proto
  COMMAND protoc -I ${LIBRFL_SRC_DIR}/rfl/proto --python_out=${libdir}/rfl/proto ${LIBRFL_SRC_DIR}/rfl/proto/reflected.proto
  COMMAND ${CMAKE_COMMAND} -E copy ${protobuf_dir}/descriptor_pb2.py ${libdir}/google/protobuf
  COMMAND ${CMAKE_COMMAND} -E make_directory ${libdir}/google/protobuf/compiler
  COMMAND protoc -I ${protobuf_dir}/src -I ${proto_dir}/compiler --python_out=${libdir}/google/protobuf/compiler ${proto_dir}/compiler/plugin.proto
  COMMAND ${CMAKE_COMMAND} -E touch ${libdir}/google/protobuf/compiler/__init__.py
  DEPENDS protoc rfl ${LIBRFL_SRC_DIR}/rfl/proto/reflected.proto
  )

install(FILES
  ${LIBRFL_SRC_DIR}/rfl-gen/rfl-dump
  ${LIBRFL_SRC_DIR}/rfl-gen/rfl-gen
  DESTINATION bin
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(DIRECTORY ${libdir}
  DESTINATION lib
  PATTERN "*.pyc" EXCLUDE
  )
