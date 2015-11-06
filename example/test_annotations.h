// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __TEST_ANNOTATIONS_H__
#define __TEST_ANNOTATIONS_H__

#include "rfl/annotations.h"
#include "example/object.h"
#include "example/test_primitives.h"
#include "example/example_export.h"

#include <vector>

namespace test {

typedef std::vector<std::string> StringArray;

struct EXAMPLE_EXPORT rfl_primitive() Vector {
};

class EXAMPLE_EXPORT rfl_class() TestBaseObject : public example::Object {
public:
  typedef std::vector<float> FloatArray;

  struct rfl_primitive() Float2 : Vector {
    float x_;
    float y_;

    Float2() : x_(0), y_(0) {}
    Float2(float x, float y) : x_(x), y_(y) {}
    Float2(Float2 const &x) : x_(x.x_), y_(x.y_) {}
    Float2 &operator=(Float2 const &x) {
      x_ = x.x_;
      y_ = x.y_;
      return *this;
    }
    bool operator==(Float2 const &x) const { return x_ == x.x_ && y_ == x.y_; }
    bool operator!=(Float2 const &x) const { return !operator==(x); }
  };


  enum rfl_enum(kDefault_Flag = "Default",
                kExtra_Flag = "Extra",
                kSpecial_Flag = "Special")
  Flags {
    kDefault_Flag,
    kExtra_Flag,
    kSpecial_Flag
  };

public:
  TestBaseObject();
  ~TestBaseObject() override;

  int int_value() const { return int_value_; }
  void set_int_value(int v) { int_value_ = v; }

  float float_value() const { return float_value_; }
  void set_float_value(float v) { float_value_ = v; }

  rfl_method(name = "Do Something")
  void DoSomething(rfl_arg(name = "A", kind = "in") int a);

  int *ptr_value() const;
  void set_ptr_value(int *ptr_value);

  int const *cptr_value() const;
  void set_cptr_value(int const *cptr_value);

  int const_int_value() const;

  TestEnum test_enum() const;
  void set_test_enum(TestEnum test_enum);

  Flags flags() const;
  void set_flags(Flags flags);

  Float2 float2() const;
  void set_float2(Float2 float2);

private:
  friend class TestBaseObjectClass;

  typedef std::vector<int> IntArray;

  rfl_property(id = "int_value", kind = "number", name = "Integer Value",
               default = 10, min = 0, max = 100,
               step = 1, page_step = 1, page_size = 1, precision = 0)
  int int_value_;

  rfl_property(id = "float_value", kind = "number", name = "Float Value",
               default = 10.0, min = 0.0, max = 100.0,
               step = .1, page_step = 1, page_size = 1, precision = 1)
  float float_value_;

  rfl_property(id = "ptr", kind = "pointer", name = "Pointer")
  int *ptr_value_;

  rfl_property(id = "cptr", kind = "pointer", name = "Const Pointer")
  int const *cptr_value_;

  rfl_property(id = "const_int_value", kind = "constant",
               name = "Const Integer Value",
               default = "0")
  int const const_int_value_;

  rfl_property(id = "test_enum", kind = "enum", name = "Test Enumeration",
               default = "kTest1")
  TestEnum test_enum_;

  rfl_property(id = "flags", kind = "enum", name = "Test Flags",
               default = "kExtra_Flag")
  Flags flags_;

  rfl_property(id = "int_array", kind = "array", name = "Integer Array",
               default = "{0, 1, 23, 45, 67}")
  IntArray int_array_;

  rfl_property(id = "float_array", kind = "array", name = "Float Array")
  FloatArray float_array_;

  rfl_property(id = "string_array", kind = "array", name = "String Array")
  StringArray string_array_;

  rfl_property(id = "float2", kind = "generic", name = "Float2",
               default = "1, 2")
  Float2 float2_;
};

class EXAMPLE_EXPORT rfl_class() TestObject : public TestBaseObject {
public:
  TestObject();
  ~TestObject() override;

  int *int_ptr() const;
  void set_int_ptr(int *ptr);

  Flags more_flags() const;
  void set_more_flags(Flags more_flags);

private:
  friend class TestObjectClass;

  typedef int *IntPtr;
  rfl_property(id = "int_ptr", kind = "pointer", name = "Integer Pointer",
               default = nullptr)
  IntPtr int_ptr_;

  rfl_property(id = "more_flags", kind = "enum", name = "More Test Flags",
               default = "kSpecial_Flag")
  Flags more_flags_;
};

class EXAMPLE_EXPORT ExternalBaseClass {
public:
  virtual ~ExternalBaseClass() {}
  virtual void ExternalStuff() = 0;
};

class EXAMPLE_EXPORT rfl_class() TestObjectExBase : public ExternalBaseClass,
                                                    public TestObject {
public:
  TestObjectExBase() {}
  ~TestObjectExBase() override {}

  // ExternalBaseClass overrides
  void ExternalStuff() override {}
};

}  // namespace test

#endif /* __TEST_ANNOTATIONS_H__ */
