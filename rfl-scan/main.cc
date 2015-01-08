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
#if 0
namespace {
  
class InsertAdjuster: public clang::tooling::ArgumentsAdjuster {
public:
  enum Position { BEGIN, END };

  InsertAdjuster(const CommandLineArguments &Extra, Position Pos)
    : Extra(Extra), Pos(Pos) {
  }

  InsertAdjuster(const char *Extra, Position Pos)
    : Extra(1, std::string(Extra)), Pos(Pos) {
  }

  virtual CommandLineArguments
  Adjust(const CommandLineArguments &Args) override {
    CommandLineArguments Return(Args);

    CommandLineArguments::iterator I;
    if (Pos == END) {
      I = Return.end();
    } else {
      I = Return.begin();
      ++I; // To leave the program name in place
    }

    Return.insert(I, Extra.begin(), Extra.end());
    return Return;
  }

private:
  const CommandLineArguments Extra;
  const Position Pos;
};

} // namespace
#endif

ArgumentsAdjuster GetInsertAdjuster(std::string const &extra) {
  return [extra] (CommandLineArguments const &args){
    CommandLineArguments ret(args);
    ret.insert(ret.end(), extra);
    return ret;
  };
}

int main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal();

  CommonOptionsParser options_parser(argc, argv, RflScanCategory);
  ClangTool tool(options_parser.getCompilations(),
                 options_parser.getSourcePathList());
  tool.clearArgumentsAdjusters();

  SmallString<128> resource_dir(LLVM_PREFIX);
  llvm::sys::path::append(resource_dir , "lib", "clang", CLANG_VERSION_STRING);

  CommandLineArguments extra_args;
#if 0
  std::string extra_flags = "-resource-dir=";
  extra_flags += resource_dir.c_str();
  extra_args.push_back(extra_flags);
#endif

  extra_args.push_back("-D__RFL_SCAN__");
  std::istringstream iss(std::string(IMPLICIT));
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter(extra_args));
  //extra_args.push_back(IMPLICIT);
  for (auto str : extra_args) {
    outs() << str << "\n";
  }
  outs().flush();

  outs() << "using resource dir: " << resource_dir.c_str() << "\n";

  tool.appendArgumentsAdjuster(
     getInsertArgumentAdjuster(extra_args, ArgumentInsertPosition::BEGIN));
     // new InsertAdjuster(rc_dir_args, InsertAdjuster::BEGIN));

  std::unique_ptr<rfl::Repository> repository(new rfl::Repository());

  std::unique_ptr<rfl::Package> package(new rfl::Package("test", "1.0"));

  // load imports
  for (std::vector<std::string>::iterator it = Imports.begin(),
                                          e = Imports.end();
       it != e;
       ++it) {
    llvm::outs() << "importing " << *it << "\n";
  }

  std::unique_ptr<rfl::scan::ASTScanActionFactory> factory(
      new rfl::scan::ASTScanActionFactory(package.get()));
  //tool.appendArgumentsAdjuster(new InsertAdjuster(IMPLICIT_INCLUDES, InsertAdjuster::BEGIN));
  int ret = tool.run(factory.get());

  //rfl::DumpPackage(std::cout, package.get());
  rfl::GeneratePackage("test_pkg.cc", package.get());
  return ret;
}
