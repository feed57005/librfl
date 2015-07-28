// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __TEST_DEPENDECIES_H__
#define __TEST_DEPENDECIES_H__

#include "example/test_annotations.h"
#include "example/example_export.h"

namespace test {

class EXAMPLE_EXPORT rfl_class(name = "Extended Test Object") ExtendedTestObject
    : public TestObject {

  void blabla();

private:
  rfl_property(id = "count",
               kind = "number",
               name = "Count",
               default = 10,
               min = 0,
               max = 100,
               step = 1,
               page_step = 1,
               page_size = 1,
               precision = 0)
  int count_;
};

} // namespace test

#endif /* __TEST_DEPENDECIES_H__ */
