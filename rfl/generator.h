// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __RFL_GENERATOR_H__
#define __RFL_GENERATOR_H__

#include "rfl/reflected.h"

#include <string>

namespace rfl {

class RFL_EXPORT Generator {
public:
  Generator();
  virtual ~Generator() {}

  void set_output_path(char const *out_path) {
    output_path_ = out_path;
  }

  void set_generate_plugin(bool generate) {
    generate_plugin_ = generate;
  }

  virtual int Generate(Package const *pkg);

protected:
  virtual int TraverseFile(PackageFile const *file);
  virtual int TraverseNamespace(Namespace const *ns,
                                PackageFile const *file = nullptr);
  virtual int TraverseClass(Class const *klass);

  virtual int BeginPackage(Package const *pkg) = 0;
  virtual int EndPackage(Package const *pkg) = 0;
  virtual int BeginFile(PackageFile const *file) = 0;
  virtual int EndFile(PackageFile const *file) = 0;
  virtual int BeginClass(Class const *klass) = 0;
  virtual int EndClass(Class const *klass) = 0;
  virtual int BeginNamespace(Namespace const *ns) = 0;
  virtual int EndNamespace(Namespace const *ns) = 0;

  std::string FilenameForPackageFile(PackageFile const *file,
                                     bool full,
                                     bool header) const;
  static std::string HeaderGuard(std::string const &name, bool begin);
  // TODO fixme should check that path exist
  static bool WriteStreamToFile(std::string const &file,
                                std::stringstream const &content);
protected:
  std::string output_path_;
  bool generate_plugin_;
  std::string h_file_suffix_;
  std::string src_file_suffix_;
};

} // namespace rfl

#endif /* __RFL_GENERATOR_H__ */
