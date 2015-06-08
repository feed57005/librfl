// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "rfl/reflected.h"

namespace rfl {

TEST(TestPackageManifest, Basic) {
  PackageManifest mf;
  mf.SetEntry("module.name", "test module");
  mf.SetEntry("module.version", "1.0");
  mf.SetEntry("module.description.short", "module");
  mf.SetEntry("module.description.long", "moduleeee");
  mf.Save("test.ini");
}

} // namespace rfl
