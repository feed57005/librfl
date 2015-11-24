set (protobuf_lite_TARGET_TYPE STATIC)
set (protobuf_lite_PUBLIC_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE INTERNAL "" FORCE)
set (protobuf_lite_SOURCES
  src/google/protobuf/extension_set.cc
  src/google/protobuf/extension_set.h
  src/google/protobuf/generated_message_util.cc
  src/google/protobuf/generated_message_util.h
  src/google/protobuf/io/coded_stream.cc
  src/google/protobuf/io/coded_stream.h
  src/google/protobuf/io/coded_stream_inl.h
  src/google/protobuf/io/zero_copy_stream.cc
  src/google/protobuf/io/zero_copy_stream.h
  src/google/protobuf/io/zero_copy_stream_impl_lite.cc
  src/google/protobuf/io/zero_copy_stream_impl_lite.h
  src/google/protobuf/message_lite.cc
  src/google/protobuf/message_lite.h
  src/google/protobuf/repeated_field.cc
  src/google/protobuf/repeated_field.h
  src/google/protobuf/stubs/atomicops.h
  src/google/protobuf/stubs/atomicops_internals_arm_gcc.h
  src/google/protobuf/stubs/atomicops_internals_atomicword_compat.h
  src/google/protobuf/stubs/atomicops_internals_macosx.h
  src/google/protobuf/stubs/atomicops_internals_mips_gcc.h
  src/google/protobuf/stubs/atomicops_internals_x86_gcc.cc
  src/google/protobuf/stubs/atomicops_internals_x86_gcc.h
  src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc
  src/google/protobuf/stubs/atomicops_internals_x86_msvc.h
  src/google/protobuf/stubs/common.cc
  src/google/protobuf/stubs/common.h
  src/google/protobuf/stubs/hash.h
  src/google/protobuf/stubs/map-util.h
  src/google/protobuf/stubs/once.cc
  src/google/protobuf/stubs/once.h
  src/google/protobuf/stubs/platform_macros.h
  src/google/protobuf/unknown_field_set.cc
  src/google/protobuf/unknown_field_set.h
  src/google/protobuf/wire_format_lite.cc
  src/google/protobuf/wire_format_lite.h
  src/google/protobuf/wire_format_lite_inl.h
)

set (protobuf_lite_PUBLIC_DEFINES
  GOOGLE_PROTOBUF_NO_RTTI
  GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  CACHE INTERNAL "" FORCE
  )

set (protobuf_lite_INCLUDE_DIRS src)
if (OS_WIN)
  list (APPEND protobuf_lite_INCLUDE_DIRS vsprojects)
else ()
  list (APPEND protobuf_lite_INCLUDE_DIRS .)
endif ()

set (protobuf_full_TARGET_TYPE STATIC)
set (protobuf_full_PUBLIC_DEFINES ${protobuf_lite_PUBLIC_DEFINES} CACHE INTERNAL "" FORCE)
set (protobuf_full_INCLUDE_DIRS ${protobuf_lite_INCLUDE_DIRS})
set (protobuf_full_PUBLIC_INCLUDE_DIRS ${protobuf_lite_PUBLIC_INCLUDE_DIRS} CACHE INTERNAL "" FORCE)
set (protobuf_full_SOURCES
  ${protobuf_lite_SOURCES}
  src/google/protobuf/compiler/code_generator.h
  src/google/protobuf/compiler/command_line_interface.h
  src/google/protobuf/compiler/importer.h
  src/google/protobuf/compiler/java/java_doc_comment.cc
  src/google/protobuf/compiler/java/java_doc_comment.h
  src/google/protobuf/compiler/parser.h
  src/google/protobuf/descriptor.cc
  src/google/protobuf/descriptor.h
  src/google/protobuf/descriptor.pb.cc
  src/google/protobuf/descriptor.pb.h
  src/google/protobuf/descriptor_database.cc
  src/google/protobuf/descriptor_database.h
  src/google/protobuf/dynamic_message.cc
  src/google/protobuf/dynamic_message.h
  src/google/protobuf/extension_set_heavy.cc
  src/google/protobuf/generated_enum_reflection.h
  src/google/protobuf/generated_message_reflection.cc
  src/google/protobuf/generated_message_reflection.h
  src/google/protobuf/io/gzip_stream.h
  src/google/protobuf/io/printer.h
  src/google/protobuf/io/tokenizer.h
  src/google/protobuf/io/zero_copy_stream_impl.h
  src/google/protobuf/message.cc
  src/google/protobuf/message.h
  src/google/protobuf/reflection_ops.cc
  src/google/protobuf/reflection_ops.h
  src/google/protobuf/service.cc
  src/google/protobuf/service.h
  src/google/protobuf/stubs/stl_util.h
  src/google/protobuf/stubs/stringprintf.cc
  src/google/protobuf/stubs/stringprintf.h
  src/google/protobuf/stubs/structurally_valid.cc
  src/google/protobuf/stubs/strutil.cc
  src/google/protobuf/stubs/strutil.h
  src/google/protobuf/stubs/substitute.cc
  src/google/protobuf/stubs/substitute.h
  src/google/protobuf/stubs/template_util.h
  src/google/protobuf/stubs/type_traits.h
  src/google/protobuf/text_format.cc
  src/google/protobuf/text_format.h
  src/google/protobuf/wire_format.cc
  src/google/protobuf/wire_format.h

  # This file pulls in zlib, but it's not actually used by protoc, so
  # instead of compiling zlib for the host, let's just exclude this.
  # src/src/google/protobuf/io/gzip_stream.cc
  src/google/protobuf/compiler/importer.cc
  src/google/protobuf/compiler/parser.cc
  src/google/protobuf/io/printer.cc
  src/google/protobuf/io/tokenizer.cc
  src/google/protobuf/io/zero_copy_stream_impl.cc
)

set (protoc_TARGET_TYPE executable)
set (protoc_SOURCES
  src/google/protobuf/compiler/code_generator.cc
  src/google/protobuf/compiler/command_line_interface.cc
  src/google/protobuf/compiler/cpp/cpp_enum.cc
  src/google/protobuf/compiler/cpp/cpp_enum.h
  src/google/protobuf/compiler/cpp/cpp_enum_field.cc
  src/google/protobuf/compiler/cpp/cpp_enum_field.h
  src/google/protobuf/compiler/cpp/cpp_extension.cc
  src/google/protobuf/compiler/cpp/cpp_extension.h
  src/google/protobuf/compiler/cpp/cpp_field.cc
  src/google/protobuf/compiler/cpp/cpp_field.h
  src/google/protobuf/compiler/cpp/cpp_file.cc
  src/google/protobuf/compiler/cpp/cpp_file.h
  src/google/protobuf/compiler/cpp/cpp_generator.cc
  src/google/protobuf/compiler/cpp/cpp_helpers.cc
  src/google/protobuf/compiler/cpp/cpp_helpers.h
  src/google/protobuf/compiler/cpp/cpp_message.cc
  src/google/protobuf/compiler/cpp/cpp_message.h
  src/google/protobuf/compiler/cpp/cpp_message_field.cc
  src/google/protobuf/compiler/cpp/cpp_message_field.h
  src/google/protobuf/compiler/cpp/cpp_primitive_field.cc
  src/google/protobuf/compiler/cpp/cpp_primitive_field.h
  src/google/protobuf/compiler/cpp/cpp_service.cc
  src/google/protobuf/compiler/cpp/cpp_service.h
  src/google/protobuf/compiler/cpp/cpp_string_field.cc
  src/google/protobuf/compiler/cpp/cpp_string_field.h
  src/google/protobuf/compiler/java/java_enum.cc
  src/google/protobuf/compiler/java/java_enum.h
  src/google/protobuf/compiler/java/java_enum_field.cc
  src/google/protobuf/compiler/java/java_enum_field.h
  src/google/protobuf/compiler/java/java_extension.cc
  src/google/protobuf/compiler/java/java_extension.h
  src/google/protobuf/compiler/java/java_field.cc
  src/google/protobuf/compiler/java/java_field.h
  src/google/protobuf/compiler/java/java_file.cc
  src/google/protobuf/compiler/java/java_file.h
  src/google/protobuf/compiler/java/java_generator.cc
  src/google/protobuf/compiler/java/java_helpers.cc
  src/google/protobuf/compiler/java/java_helpers.h
  src/google/protobuf/compiler/java/java_message.cc
  src/google/protobuf/compiler/java/java_message.h
  src/google/protobuf/compiler/java/java_message_field.cc
  src/google/protobuf/compiler/java/java_message_field.h
  src/google/protobuf/compiler/java/java_primitive_field.cc
  src/google/protobuf/compiler/java/java_primitive_field.h
  src/google/protobuf/compiler/java/java_service.cc
  src/google/protobuf/compiler/java/java_service.h
  src/google/protobuf/compiler/java/java_string_field.cc
  src/google/protobuf/compiler/java/java_string_field.h
  src/google/protobuf/compiler/main.cc
  src/google/protobuf/compiler/plugin.cc
  src/google/protobuf/compiler/plugin.pb.cc
  src/google/protobuf/compiler/python/python_generator.cc
  src/google/protobuf/compiler/subprocess.cc
  src/google/protobuf/compiler/subprocess.h
  src/google/protobuf/compiler/zip_writer.cc
  src/google/protobuf/compiler/zip_writer.h
)
set (protoc_DEPS protobuf_full)
