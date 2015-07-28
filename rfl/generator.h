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
  virtual ~Generator();

  virtual int Generate(Package const *pkg);

  char const *output_path() const;
  void set_output_path(char const *out_path);

  char const *output_file() const;
  void set_output_file(char const *out_file);

  void set_generate_plugin(bool generate);
  bool generate_plugin() const;

  size_t GetNumGeneratedFiles() const;
  char const *GetGeneratedFileAt(size_t idx) const;
  void AddGeneratedFile(char const *file);
  void RemoveGeneratedFile(char const *file);

  static bool WriteToFile(char const *file, char const *data);

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

private:
  std::vector<std::string> generated_files_;
  std::string output_path_;
  std::string output_file_;
  bool generate_plugin_;
};

} // namespace rfl

#endif /* __RFL_GENERATOR_H__ */
