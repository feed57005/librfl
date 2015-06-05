#ifndef __TEST_ANNOTATIONS_H__
#define __TEST_ANNOTATIONS_H__

#include "rfl/annotations.h"
#include "object.h"

namespace test {

class TestBaseObject;

enum rfl_enum(name = "Test Enum",
              kTest1 = "Test One",
              kTest2 = "Test Two") TestEnum {
  kTest1 = 0,
  kTest2,
};

class rfl_class(name = "Test Base Object",
                package = "test_annotations") TestBaseObject
    : public test::Object {
public:
  TestBaseObject();
  ~TestBaseObject();

  int int_value() const { return int_value_; }
  void set_int_value(int v) { int_value_ = v; }

  float float_value() const { return float_value_; }
  void set_float_value(float v) { float_value_ = v; }

  rfl_method(name = "Do Something") void DoSomething(
      rfl_arg(name = "A", kind = "in") int a);

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

  rfl_property(id = "float_value",
               kind = "number",
               name = "Float Value",
               default = 10.0,
               min = 0.0,
               max = 100.0,
               step = .1,
               page_step = 1,
               page_size = 1,
               precision = 1) float float_value_;

  rfl_property(id = "ptr",
               kind = "pointer",
               name = "Pointer",
               default = "nullptr") int *ptr_value_;

  rfl_property(id = "cptr",
               kind = "pointer",
               name = "Const Pointer",
               default = "nullptr") int const *cptr_value_;

  rfl_property(id = "enumeration",
               kind = "enum",
               name = "Enumeration",
               default = "kTest1") TestEnum test_enum_;
};

class rfl_class(name = "Test Object", package = "test_annotations") TestObject
    : public TestBaseObject {
public:
  TestObject();
  ~TestObject();

private:
};

}  // namespace test

#endif /* __TEST_ANNOTATIONS_H__ */
