// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "example/test_annotations.h"
#include "test_annotations.rfl.h"

#include <iostream>
#include <cstddef>

namespace test {

static int g_Global = 22;

TestBaseObject::TestBaseObject()
    : int_value_(23),
      float_value_(23.23),
      ptr_value_(&g_Global),
      cptr_value_(&g_Global),
      const_int_value_(23) {
}

TestBaseObject::~TestBaseObject() {
}

void TestBaseObject::DoSomething(int a) {
  int_value_ += a;
}

int *TestBaseObject::ptr_value() const {
  return ptr_value_;
}

void TestBaseObject::set_ptr_value(int *ptr_value) {
  ptr_value_ = ptr_value;
}

int const *TestBaseObject::cptr_value() const {
  return cptr_value_;
}

void TestBaseObject::set_cptr_value(int const *cptr_value) {
  cptr_value_ = cptr_value;
}

int TestBaseObject::const_int_value() const {
  return const_int_value_;
}

TestEnum TestBaseObject::test_enum() const {
  return test_enum_;
}

void TestBaseObject::set_test_enum(TestEnum test_enum) {
  test_enum_ = test_enum;
}

TestBaseObject::Flags TestBaseObject::flags() const {
  return flags_;
}

void TestBaseObject::set_flags(Flags flags) {
  flags_ = flags;
}

TestBaseObject::Float2 TestBaseObject::float2() const {
  return float2_;
}

void TestBaseObject::set_float2(Float2 float2) {
  float2_ = float2;
}

////////////////////////////////////////////////////////////////////////////////

TestObject::TestObject() : int_ptr_(nullptr), more_flags_(kSpecial_Flag) {}

TestObject::~TestObject() {}

int *TestObject::int_ptr() const {
  return int_ptr_;
}

void TestObject::set_int_ptr(int *ptr) {
  int_ptr_ = ptr;
}

TestBaseObject::Flags TestObject::more_flags() const {
  return more_flags_;
}

void TestObject::set_more_flags(Flags more_flags) {
  more_flags_ = more_flags;
}

} // namespace test
