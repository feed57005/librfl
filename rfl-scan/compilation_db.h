#ifndef __RFL_SCAN_COMPILATION_DB_H__
#define __RFL_SCAN_COMPILATION_DB_H__

#include "clang/Tooling/CompilationDatabase.h"

namespace rfl {
namespace scan {

using namespace llvm;
using namespace clang::tooling;

// CompilationDatabase that tries to use flags of .cc/.cpp/.cxx/.c++ of when
// requesting a header file
// It's a workaround for CMake, that does not generate
// compile commands for header files
class ScanCompilationDatabase : public CompilationDatabase {
public:
  ScanCompilationDatabase(CompilationDatabase &cdb,
                          std::vector<std::string> sources);
  virtual ~ScanCompilationDatabase();

  virtual std::vector<CompileCommand> getCompileCommands(
      StringRef FilePath) const;
  virtual std::vector<std::string> getAllFiles() const;
  virtual std::vector<CompileCommand> getAllCompileCommands() const;

private:
	void cleanupCommands(std::vector<CompileCommand> &cmds, StringRef const &filename) const;
  CompilationDatabase &compilation_db_;
  std::vector<std::string> source_files_;
};

} // namespace scan
} // namespace rfl

#endif /* __RFL_SCAN_COMPILATION_DB_H__ */
