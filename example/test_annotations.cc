#include "test_annotations.h"

#include <iostream>
#include <cstddef>

namespace test {
static int g_Global = 22;

TestObject::TestObject() {}

TestObject::~TestObject() {}

//rfl_attr(constructor_def)
TestBaseObject::TestBaseObject()
    : int_value_(23),
      float_value_(23.23),
      ptr_value_(nullptr),
      cptr_value_(nullptr)
      /*ref_value_(g_Global),
      cref_value_(g_Global) */{
}

//rfl_attr(destructor_def)
TestBaseObject::~TestBaseObject() {
}

void TestBaseObject::DoSomething(int a) {
}

} // namespace test
