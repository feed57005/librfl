// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __EXAMPLE_ENUM_CLASS_H__
#define __EXAMPLE_ENUM_CLASS_H__

#include "example/example_export.h"
#include "example/type_class.h"

#include <vector>

namespace example {

class EXAMPLE_EXPORT EnumClass : public TypeClass {
public:
  struct Item {
    char const *id;
    char const *name;
    long value;

    Item() : id(nullptr), name(nullptr), value(0) {}

    Item(char const *_id, char const *_name, long _value)
        : id(_id), name(_name), value(_value) {}

    Item(Item const &x) : id(x.id), name(x.name), value(x.value) {}

    Item &operator=(Item const &x) {
      id = x.id;
      name = x.name;
      value = x.value;
      return *this;
    }
  };

public:
  EnumClass(char const *name) : TypeClass(name) {}

  ~EnumClass() override {}

  Kind GetTypeClassKind() const override {
    return ENUM_KIND;
  }

  char const *enum_name() const {
    return enum_name_;
  }

  Item const *item_at(int i) const {
    return &items_[i];
  }
  int item_num() const {
    return (int) items_.size();
  }
  void AddItem(Item const &item) {
    items_.push_back(item);
  }

protected:
  TypeId enum_id_;
  char const *enum_name_;
  std::vector<Item> items_;
};

} // namespace example

#endif /* __EXAMPLE_ENUM_CLASS_H__ */
