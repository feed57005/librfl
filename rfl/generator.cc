// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rfl/generator.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cerrno>

namespace rfl {

Generator::Generator() {
}

Generator::~Generator() {
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

  ret = EndPackage(pkg);
  if (ret)
    return ret;

  // write output files listing
  if (!output_file_.empty()) {
    std::stringstream os;
    for (std::string const &file : generated_files_) {
      os << file << "\n";
    }
    if (!WriteToFile(output_file_.c_str(), os.str().c_str()))
      return 1;
  }
  return 0;
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

size_t Generator::GetNumGeneratedFiles() const {
  return generated_files_.size();
}

char const *Generator::GetGeneratedFileAt(size_t idx) const {
  return generated_files_[idx].c_str();
}

void Generator::AddGeneratedFile(char const *file) {
  std::string const filename(file);
  std::vector<std::string>::const_iterator found =
    std::find(generated_files_.begin(), generated_files_.end(), filename);
  if (found == generated_files_.end())
    generated_files_.push_back(filename);
}

void Generator::RemoveGeneratedFile(char const *file) {
  std::string const filename(file);
  std::vector<std::string>::iterator found =
    std::find(generated_files_.begin(), generated_files_.end(), filename);
  if (found != generated_files_.end())
    generated_files_.erase(found);
}

char const *Generator::output_path() const {
  return output_path_.c_str();
}

void Generator::set_output_path(char const *out_path) {
  output_path_ = out_path;
}

char const *Generator::output_file() const {
  return output_file_.c_str();
}

void Generator::set_output_file(char const *out_file) {
  output_file_ = out_file;
}

void Generator::set_generate_plugin(bool generate) {
  generate_plugin_ = generate;
}

bool Generator::generate_plugin() const {
  return generate_plugin_;
}

bool Generator::WriteToFile(char const *file, char const *data) {
  using namespace llvm::sys;

  llvm::SmallString<256> path(file);
  path::remove_filename(path);
  if (fs::create_directories(path, true)) {
    return false;
  }
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
  file_out << data;
  file_out.close();
  return true;
}

} // namespace rfl
