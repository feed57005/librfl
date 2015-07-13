// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __TEST_PRIMITIVES_H__
#define __TEST_PRIMITIVES_H__

#include "rfl/annotations.h"

namespace test {

typedef rfl_primitive(name = "Test Int Type")
int TestInt;

enum rfl_enum(name = "Test Enum",
              kTest1 = "Test One",
              kTest2 = "Test Two")
TestEnum {
  kTest1 = 0,
  kTest2,
};

} // namespace test

#endif /* __TEST_PRIMITIVES_H__ */
