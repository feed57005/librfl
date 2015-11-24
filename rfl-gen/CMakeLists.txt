set(outdir ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/bin/rfl-gen)
set(protobuf_dir ${CMAKE_SOURCE_DIR}/third_party/protobuf)
set(proto_dir ${protobuf_dir}/src/google/protobuf)

add_custom_target(rfl-gen ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${outdir}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/rfl-gen/rfl-gen.py ${outdir}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/rfl-gen/rfl-dump.py ${outdir}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/rfl-gen/rfl ${outdir}/rfl
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/rfl-gen/templates ${outdir}/templates
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/third_party/jinja2/jinja2 ${outdir}/jinja2
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/third_party/protobuf/python/google ${outdir}/google
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/rfl/reflected_pb2.py ${outdir}/rfl/proto
  COMMAND protoc -I ${CMAKE_SOURCE_DIR}/rfl/proto --python_out=${outdir}/rfl/proto ${CMAKE_SOURCE_DIR}/rfl/proto/reflected.proto
  COMMAND ${CMAKE_COMMAND} -E copy ${protobuf_dir}/descriptor_pb2.py ${outdir}/google/protobuf
  COMMAND ${CMAKE_COMMAND} -E make_directory ${outdir}/google/protobuf/compiler
  COMMAND protoc -I ${protobuf_dir}/src -I ${proto_dir}/compiler --python_out=${outdir}/google/protobuf/compiler ${proto_dir}/compiler/plugin.proto
  COMMAND ${CMAKE_COMMAND} -E touch ${outdir}/google/protobuf/compiler/__init__.py
  DEPENDS protoc ${CMAKE_SOURCE_DIR}/rfl/proto/reflected.proto
  )

install(DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin/rfl-gen
  DESTINATION bin/rfl-gen
  PATTERN "*.pyc" EXCLUDE
  )
