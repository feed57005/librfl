#ifndef __TEST_ANNOTATIONS_H__
#define __TEST_ANNOTATIONS_H__

#include "rfl/annotations.h"

namespace test {

class TestBaseObject;

static int g_Global = 23;

class rfl_class(name = "Test Object Base") TestBaseObject {
public:
  TestBaseObject();
  ~TestBaseObject();

  int int_value() const { return int_value_; }
  void set_int_value(int v) { int_value_ = v; }

  float float_value() const { return float_value_; }
  void set_float_value(float v) { float_value_ = v; }

  void DoSomething(int a);

private:
  rfl_property(id = "int_value",
               kind = "number",
               name = "Integer Value",
               default = 10,
               min = 0,
               max = 100,
               step = 1,
               page_step = 1,
               page_size = 1,
               precision = 0) int int_value_;

  rfl_property(id="float_value",
               kind = "number",
               name = "Float Value",
               default = 10.0,
               min = 0.0,
               max = 100.0,
               step = .1,
               page_step = 1,
               page_size = 1,
               precision = 1) float float_value_;

  rfl_property(id="ptr",
               kind="pointer",
               name="Pointer") int *ptr_value_;

  rfl_property(id="cptr",
               kind="pointer",
               name="Const Pointer") int const *cptr_value_;

  rfl_property(id="ref",
               kind="reference",
               name="Reference") int &ref_value_;

  rfl_property(id="cref"
               kind="reference",
               name="Const Reference") int const &cref_value_;

  rfl_property(id="int4_array",
               kind="array",
               name="Int4 Array") int int4_array_[4];

};

class rfl_class(name = "Test Object") TestObject : public TestBaseObject {
public:
  TestObject();
  ~TestObject();

private:
};

} // namespace test

#endif /* __TEST_ANNOTATIONS_H__ */
