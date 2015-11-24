// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __EXAMPLE_PACKAGE_H__
#define __EXAMPLE_PACKAGE_H__

#include "example/example_export.h"
#include "rfl/native_library.h"

#include <string>

namespace example {

class TypeRepository;

class EXAMPLE_EXPORT Package {
public:
  virtual ~Package() {}

  virtual bool RegisterPackage(TypeRepository *repo) = 0;
  virtual bool UnregisterPackage(TypeRepository *repo) = 0;

  rfl::NativeLibrary native_lib() const {
    return native_lib_;
  }

  std::string const &name() const {
    return package_name_;
  }

protected:
  friend class TypeRepository;
  std::string package_name_;
  rfl::NativeLibrary native_lib_;
};

} // namespace example

#endif /* __EXAMPLE_PACKAGE_H__ */
