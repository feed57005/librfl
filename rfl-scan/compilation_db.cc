// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rfl-scan/compilation_db.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_os_ostream.h"

namespace rfl {
namespace scan {

ScanCompilationDatabase::ScanCompilationDatabase(
    CompilationDatabase &cdb,
    std::vector<std::string> sources) : compilation_db_(cdb), source_files_(sources){
}

ScanCompilationDatabase::~ScanCompilationDatabase() {
}

char const *const kHeaderExtensions[] = {".h", ".hh", ".hpp", ".h++"};
char const *const kSourceExtensions[] = {".cc", ".cpp", ".cxx", ".c++", ".c"};

void ScanCompilationDatabase::cleanupCommands(std::vector<CompileCommand> &cmds,
                                              StringRef const &filename) const {
  // go through all commandlines and replace -c file with ours
  for (CompileCommand &cmd : cmds) {
    for (std::vector<std::string>::iterator it = cmd.CommandLine.begin();
         it != cmd.CommandLine.end();) {
      if (!it->empty() && it->compare("-c") == 0) {
        ++it;
        it->assign(filename);
      } else if (it->compare("-gsplit-dwarf") == 0) {
        it = cmd.CommandLine.erase(it);
      }
      else {
        ++it;
      }
    }
    // since this is header we need to specify that's a c++ header
    cmd.CommandLine.insert(cmd.CommandLine.begin() + 1, "c++");
    cmd.CommandLine.insert(cmd.CommandLine.begin() + 1, "-x");
  }
}

std::vector<CompileCommand> ScanCompilationDatabase::getCompileCommands(
    StringRef file) const {
  std::vector<CompileCommand> result = compilation_db_.getCompileCommands(file);
  if (!result.empty())
    return result;

  // check whether this is a header, if so, try to find .cc file
  StringRef ext = sys::path::extension(file);
  ArrayRef<char const *> hdr_exts(kHeaderExtensions);
  ArrayRef<char const *> src_exts(kSourceExtensions);
  for (ArrayRef<char const *>::const_iterator it = hdr_exts.begin();
       it != hdr_exts.end();
       ++it) {
    if (ext.compare(*it) == 0) {
      // header extensions match, so try to replace it with source ext.
      for (ArrayRef<char const *>::const_iterator cit = src_exts.begin();
           cit != src_exts.end();
           ++cit) {
        SmallString<1024> src_file(file);
        sys::path::replace_extension(src_file, *cit);
        result = compilation_db_.getCompileCommands(src_file);
        if (result.size()) {
          cleanupCommands(result,file);
          return result;
        }
      }
      break;
    }
  }

  outs() << "No luck for: " << file << "\n";
  outs().flush();

  // Nothing found, try all other files
  for (std::string const &src : source_files_) {
    if (src.compare(file) == 0)
      continue;
    result = getCompileCommands(src);
    if (!result.empty())
      cleanupCommands(result,file);
      return result;
  }
  return result;
}

std::vector<std::string> ScanCompilationDatabase::getAllFiles() const {
  return source_files_;
}

std::vector<CompileCommand> ScanCompilationDatabase::getAllCompileCommands() const {
  std::vector<CompileCommand> cmds;
  for (std::string const &source : source_files_) {
    std::vector<CompileCommand> source_cmds =
        compilation_db_.getCompileCommands(source);
    if (!source_cmds.empty())
      cmds.insert(cmds.end(), source_cmds.begin(), source_cmds.end());
  }
  return cmds;
}

} // namespace scan
} // namespace rfl
