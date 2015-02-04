#include "clang/Frontend/FrontendActions.h"
#include "clang/Driver/Options.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Basic/Version.h"

#include "llvm/Config/llvm-config.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallString.h"

#include "rfl/reflected.h"
#include "rfl/repository.h"
#include "rfl/reflected_io.h"
#include "rfl/native_library.h"
#include "rfl-scan/ast_scan.h"

#include <iostream>
#include <sstream>
#include <algorithm>

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...");

static cl::OptionCategory RflScanCategory("rfl-scan options");
static std::unique_ptr<opt::OptTable> Options(createDriverOptTable());
static cl::list<std::string> Generators("G",
                                        cl::desc("Specify output generator"),
                                        cl::value_desc("generator-name"),
                                        cl::cat(RflScanCategory));
static cl::list<std::string> Imports("i",
                                     cl::Prefix, cl::ZeroOrMore,
                                     cl::desc("import rfl library"),
                                     cl::cat(RflScanCategory));

static cl::opt<std::string> OutputName("output",
                                      cl::desc("output file name prefix"),
                                      cl::cat(RflScanCategory));
static cl::opt<std::string> PackageName("pkg-name",
                                      cl::desc("package name"),
                                      cl::cat(RflScanCategory));
static cl::opt<std::string> PackageVersion("pkg-version",
                                      cl::desc("package version"),
                                      cl::cat(RflScanCategory));
static cl::opt<std::string> Basedir("basedir",
                                      cl::desc("package basedir"),
                                      cl::cat(RflScanCategory));

ArgumentsAdjuster GetInsertAdjuster(std::string const &extra) {
  return [extra] (CommandLineArguments const &args){
    CommandLineArguments ret(args);
    ret.insert(ret.end(), extra);
    return ret;
  };
}

typedef int (*GenPackage)(char const *path, rfl::Package *pkg);

int main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal();

  CommonOptionsParser options_parser(argc, argv, RflScanCategory);
  ClangTool tool(options_parser.getCompilations(),
                 options_parser.getSourcePathList());
  tool.clearArgumentsAdjusters();

  SmallString<128> resource_dir(LLVM_PREFIX);
  llvm::sys::path::append(resource_dir , "lib", "clang", CLANG_VERSION_STRING);

  CommandLineArguments extra_args;

  extra_args.push_back("-D__RFL_SCAN__");
  std::istringstream iss(std::string(IMPLICIT));
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter(extra_args));

  for (auto str : extra_args) {
    outs() << str << "\n";
  }
  outs().flush();

  outs() << "using resource dir: " << resource_dir.c_str() << "\n";

  tool.appendArgumentsAdjuster(
     getInsertArgumentAdjuster(extra_args, ArgumentInsertPosition::BEGIN));

  std::unique_ptr<rfl::Repository> repository(new rfl::Repository());

  std::string const &pkg_name = PackageName.getValue();
  std::string const &pkg_version = PackageVersion.getValue();
  std::unique_ptr<rfl::Package> package(new rfl::Package(pkg_name.c_str(), pkg_version.c_str()));

  // load imports
  for (std::vector<std::string>::iterator it = Imports.begin(),
                                          e = Imports.end();
       it != e;
       ++it) {
    llvm::outs() << "importing " << *it << "\n";
  }

  std::string const &basedir = Basedir.getValue();
  std::unique_ptr<rfl::scan::ASTScannerContext> scan_ctx(
    new rfl::scan::ASTScannerContext(repository.get(), package.get(), Basedir.getValue())
    );

  std::unique_ptr<rfl::scan::ASTScanActionFactory> factory(
      new rfl::scan::ASTScanActionFactory(scan_ctx.get()));
  int ret = tool.run(factory.get());

  if (Generators.empty()) {
    std::string output_name = OutputName.getValue();
    output_name+= ".cc";
    rfl::GeneratePackage(output_name.c_str(), package.get());
  } else {
    for (std::string const &generator : Generators) {
      std::string err;
      rfl::NativeLibrary lib = rfl::LoadNativeLibrary(generator.c_str(), &err);
      if (!lib) {
        llvm::errs() << err;
        continue;
      }
      GenPackage func = (GenPackage)rfl::GetFunctionPointerFromNativeLibrary(
          lib, "GeneratePackage");
      if (func == nullptr) {
        llvm::errs() << "Could not find symbol GeneratePackage";
        continue;
      }
      func(OutputName.getValue().c_str(), package.get());
    }
  }
  return ret;
}
