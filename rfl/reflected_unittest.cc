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
