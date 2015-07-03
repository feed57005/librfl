// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rfl/generator.h"

#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cerrno>

namespace rfl {


std::string Generator::FilenameForPackageFile(PackageFile const *file, bool full, bool header) const {
  std::string ret = full ? file->source_path() : file->filename();
  if (header)
    ret += h_file_suffix_;
  else
    ret += src_file_suffix_;
  return ret;
}

// static
bool Generator::WriteStreamToFile(std::string const &file,
                            std::stringstream const &content) {
  // TODO check whether file exists, if so compare it and write a new file
  // only when they differ
  std::ofstream file_out;
  file_out.open(file);
  if (!file_out.good()) {
    std::cerr << "Failed to open file " << file << " " << std::strerror(errno)
              << std::endl;
    file_out.close();
    return false;
  }
  file_out << content.str();
  file_out.close();
  return true;
}

// static
std::string Generator::HeaderGuard(std::string const &name, bool begin) {
  std::stringstream hguard;
  std::string id = name;
  std::replace(id.begin(), id.end(), '.', '_');
  std::replace(id.begin(), id.end(), '/', '_');
  std::replace(id.begin(), id.end(), ' ', '_');
  std::transform(id.begin(), id.end(), id.begin(), ::toupper);
  if (begin) {
    hguard << "#ifndef __" << id << "_RFL_H__\n"
           << "#define __" << id << "_RFL_H__\n\n";
  } else {
    hguard << "#endif // __" << id << "_RFL_H_\n";
  }
  return hguard.str();
}

Generator::Generator() : h_file_suffix_(".rfl.h"), src_file_suffix_(".rfl.cc") {
}

int Generator::Generate(Package const *pkg) {
  int ret = BeginPackage(pkg);
  if (ret)
    return ret;

  for (size_t i = 0; i < pkg->GetNumPackageFiles(); ++i) {
    PackageFile *file = pkg->GetPackageFileAt(i);
    if (BeginFile(file))
      continue;
    ret = TraverseFile(file);
    ret = EndFile(file);
  }

  return EndPackage(pkg);
}

int Generator::TraverseFile(PackageFile const *file) {
  // collect root namespaces (ignoring package)
  std::set<Namespace *> nss;
  for (size_t i = 0; i < file->GetNumClasses(); ++i) {
    Class *klass = file->GetClassAt(i);
    Namespace *ns = klass->class_namespace();
    while (ns && ns->parent_namespace() &&
           ns->parent_namespace()->parent_namespace()) {
      ns = ns->parent_namespace();
    }
    if (ns) {
      nss.insert(ns);
    }
  }
  for (Namespace *ns : nss) {
    if (TraverseNamespace(ns, file))
      return 1;
  }
  return 0;
}

int Generator::TraverseNamespace(Namespace const *ns,
                                 PackageFile const *filter_file) {
  int ret = BeginNamespace(ns);
  for (size_t i = 0; i < ns->GetNumNamespaces(); i++) {
    TraverseNamespace(ns->GetNamespaceAt(i), filter_file);
  }
  for (size_t i = 0; i < ns->GetNumClasses(); i++) {
    Class *clazz = ns->GetClassAt(i);
    if (filter_file && filter_file != clazz->package_file())
      continue;
    ret = TraverseClass(clazz);
    if (ret)
      return ret;
  }
  ret = EndNamespace(ns);
  return ret;
}

int Generator::TraverseClass(Class const *klass) {
  int ret = BeginClass(klass);
  for (size_t i = 0; i < klass->GetNumClasses(); i++) {
    ret = TraverseClass(klass->GetClassAt(i));
    if (ret)
      return ret;
  }
  return EndClass(klass);
}

} // namespace rfl
