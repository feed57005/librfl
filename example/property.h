// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __EXAMPLE_PROPERTY_H__
#define __EXAMPLE_PROPERTY_H__

#include "example/example_export.h"
#include "example/type_class.h"
#include "example/any_var.h"
#include "example/enum_class.h"

namespace example {

using rfl::uint32;

class Object;

class EnumClass;

class EXAMPLE_EXPORT PropertySpec {
public:
  PropertySpec(char const *name,
               char const *human_name,
               TypeId cid,
               AnyVar const &default_value);

  char const *name() const { return name_; }
  char const *human_name() const { return human_name_; }
  TypeId class_id() const { return class_id_; }
  AnyVar const &default_value() const { return default_value_; }

  template <typename T>
  void Set(Object *obj,
           T const &new_value,
           uint32 offset,
           bool check = true) const {
    T *value = (T *)((char *)obj + (size_t)offset);
    T tmp = *value;
    *value = new_value;
    if (check && !validate(obj, offset)) {
      *value = tmp;
    }
  }

  template <typename T>
  T const &Get(Object *obj, uint32 offset) const {
    T const *value = (T *)((char *)obj + (size_t)offset);
    return *value;
  }

protected:
  virtual bool validate(Object *, uint32 offset) const {
    return true;
  }

  char const *name_;
  char const *human_name_;
  TypeId class_id_;
  AnyVar default_value_;
};

template <typename T>
class NumericPropertySpec : public PropertySpec {
public:
  NumericPropertySpec(char const *name,
                  char const *human_name,
                  TypeId cid,
                  AnyVar const &def_value,
                  T min,
                  T max,
                  T step,
                  T page_step,
                  T page_size,
                  int precision)
      : PropertySpec(name, human_name, cid, def_value),
        min_(min),
        max_(max),
        step_(step),
        page_step_(page_step),
        page_size_(page_size),
        precision_(precision)
  {}

 T min() const { return min_; }
 T max() const { return max_; }
 T step() const { return step_; }
 T page_step() const { return page_step_; }
 T page_size() const { return page_size_; }
 int precision() const { return precision_; }

protected:
  bool validate(Object *obj, uint32 offset) const override {
    T const &value = Get<T>(obj, offset);
    if (value >= min_ && value < max_)
      return true;
    return false;
  }

  T const min_;
  T const max_;
  T const step_;
  T const page_step_;
  T const page_size_;
  int const precision_;
};

class EXAMPLE_EXPORT EnumPropertySpec : public PropertySpec {
public:
  EnumPropertySpec(EnumClass *enm,
                   char const *name,
                   char const *human_name,
                   long default_value)
      : PropertySpec(name, human_name, enm->type_id(), AnyVar(default_value)),
        enum_(enm) {}

  EnumClass *enum_class() const { return enum_; }

private:
  EnumClass *enum_;
};

class EXAMPLE_EXPORT Property {
public:
  Property();
  Property(PropertySpec *spec, uint32 offset);
  Property(Property const &x);
  Property &operator=(Property const &x);

  uint32 offset() const;
  PropertySpec const *spec() const;

  template <typename T>
  void Set(Object *obj, T const &new_value, bool check = true) const {
    property_spec_->Set<T>(obj, new_value, offset_, check);
  }

  template <typename T>
  T const &Get(Object *obj) const {
    return property_spec_->Get<T>(obj, offset_);
  }

private:
  PropertySpec *property_spec_;
  uint32 offset_;
};

} // namespace example

#endif /* __EXAMPLE_PROPERTY_H__ */
