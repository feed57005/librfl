// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "example/test_annotations.h"
#include "test_annotations.rfl.h"

#include <iostream>
#include <cstddef>

namespace test {

static int g_Global = 22;

TestObject::TestObject() {}

TestObject::~TestObject() {}

TestBaseObject::TestBaseObject()
    : int_value_(23),
      float_value_(23.23),
      ptr_value_(nullptr),
      cptr_value_(nullptr),
      const_int_value_(23) {
}

TestBaseObject::~TestBaseObject() {
}

void TestBaseObject::DoSomething(int a) {
}

} // namespace test
