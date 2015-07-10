// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __TEST_GENERATOR_H__
#define __TEST_GENERATOR_H__

#include "rfl/reflected.h"
#include "rfl/generator.h"

#include <string>
#include <vector>
#include <sstream>

namespace rfl {

class Gen : public Generator {
public:
  virtual ~Gen() {}

protected:
  virtual int BeginPackage(Package const *pkg);
  virtual int EndPackage(Package const *pkg);
  virtual int BeginFile(PackageFile const *file);
  virtual int EndFile(PackageFile const *file);
  virtual int BeginClass(Class const *klass);
  virtual int EndClass(Class const *klass);
  virtual int BeginNamespace(Namespace const *ns);
  virtual int EndNamespace(Namespace const *ns);

  void AddInclude(std::string const &inc, std::vector<std::string> &includes);
protected:

  std::stringstream out_;
  std::stringstream hout_;
  Package const *generated_package_;
  PackageFile const *generated_file_;

  std::string package_upper_;
  std::string header_prologue_;

  std::vector<std::string> pkg_includes_;

  std::vector<std::string> src_includes_;
  std::vector<std::string> h_includes_;
  std::vector<std::pair<std::string, std::string> > classes_;
  std::vector<Enum *> enums_;
};

} // namespace rfl

#endif /* __TEST_GENERATOR_H__ */
