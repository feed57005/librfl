// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
#include "rfl/generator.h"
#include "rfl/native_library.h"
#include "rfl-scan/ast_scan.h"
#include "rfl-scan/compilation_db.h"

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
                                        cl::desc("Specify output Generator plugin"),
                                        cl::value_desc("generator-name"),
                                        cl::cat(RflScanCategory));
static cl::list<std::string> Imports("i",
                                     cl::Prefix,
                                     cl::ZeroOrMore,
                                     cl::desc("Import rfl library"),
                                     cl::cat(RflScanCategory));
static cl::list<std::string> Libs("l",
                                  cl::Prefix,
                                  cl::ZeroOrMore,
                                  cl::desc("Link library"),
                                  cl::cat(RflScanCategory));

static cl::opt<std::string> OutputName("output",
                                       cl::desc("Output file name prefix"),
                                       cl::cat(RflScanCategory));
static cl::opt<std::string> PackageName("pkg-name",
                                        cl::desc("Package name"),
                                        cl::cat(RflScanCategory));
static cl::opt<std::string> PackageVersion("pkg-version",
                                           cl::desc("Package version"),
                                           cl::cat(RflScanCategory));
static cl::opt<std::string> Basedir("basedir",
                                    cl::desc("Package basedir"),
                                    cl::cat(RflScanCategory));

static cl::opt<bool> GeneratePlugin("plugin",
                                    cl::desc("Generate pluging"),
                                    cl::cat(RflScanCategory));
static cl::opt<unsigned> Verbose("verbose",
                                 cl::desc("Verbose level"),
                                 cl::init(0),
                                 cl::cat(RflScanCategory));

static std::string StripBasedir(std::string const &filename,
                                std::string const &basedir) {
  size_t basedir_pos = filename.find(basedir);
  if (basedir_pos != std::string::npos && basedir_pos == 0) {
    return filename.substr(basedir.length() + 1,
                           filename.length() - basedir.length());
  }
  return filename;
}

ArgumentsAdjuster GetInsertAdjuster(std::string const &extra) {
  return [extra] (CommandLineArguments const &args){
    CommandLineArguments ret(args);
    ret.insert(ret.end(), extra);
    return ret;
  };
}

typedef int (*GenPackage)(char const *path, rfl::Package *pkg);
typedef rfl::Generator *(*CreateGenerator)();

int main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal();

  CommonOptionsParser options_parser(argc, argv, RflScanCategory);

  std::vector<std::string> source_path_list =
      options_parser.getSourcePathList();
  CompilationDatabase &default_cdb = options_parser.getCompilations();
  rfl::scan::ScanCompilationDatabase cdb(default_cdb, source_path_list);
  ClangTool tool(cdb, source_path_list);
  tool.clearArgumentsAdjusters();

  SmallString<128> resource_dir(LLVM_PREFIX);
  llvm::sys::path::append(resource_dir , "lib", "clang", CLANG_VERSION_STRING);

  CommandLineArguments extra_args;

  std::string pkg_upper = PackageName.getValue();
  std::transform(pkg_upper.begin(), pkg_upper.end(), pkg_upper.begin(), ::toupper);

  // TODO why is this here?
  extra_args.push_back(
      Twine("-D")
          .concat(Twine(pkg_upper).concat("_IMPLEMENTATION"))
          .str());

  extra_args.push_back("-D__RFL_SCAN__");
  extra_args.push_back("-Wno-unused-local-typedef"); // TODO hack

  // split and insert IMPLICIT includes
  std::istringstream iss(std::string(IMPLICIT));
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter(extra_args));

  if (Verbose.getValue()) {
    for (auto str : extra_args) {
      outs() << str << "\n";
    }
    outs().flush();
  }

  if (Verbose.getValue())
    outs() << "using resource dir: " << resource_dir.c_str() << "\n";

  tool.appendArgumentsAdjuster(
     getInsertArgumentAdjuster(extra_args, ArgumentInsertPosition::BEGIN));

  std::string const &output_name = OutputName.getValue();
  std::string const &basedir = Basedir.getValue();
  std::string const &pkg_name = PackageName.getValue();
  std::string const &pkg_version = PackageVersion.getValue();
  std::unique_ptr<rfl::Package> package(new rfl::Package(pkg_name.c_str(), pkg_version.c_str()));

  // handle imports
  for (std::vector<std::string>::iterator it = Imports.begin(),
                                          e = Imports.end();
       it != e; ++it) {
    llvm::outs() << "importing " << *it << "\n";
    package->AddImport(*it);
  }

  // handle libs
  for (std::vector<std::string>::iterator it = Libs.begin(), e = Libs.end();
       it != e; ++it) {
    package->AddLibrary(*it);
  }

  // handle sources
  for (std::string const &src : source_path_list) {
    std::string rel_src = StripBasedir(src, Basedir.getValue());
    rfl::PackageFile *pkg_file = package->GetOrCreatePackageFile(rel_src);
    pkg_file->set_is_dependecy(false);
  }

  std::unique_ptr<rfl::scan::ASTScannerContext> scan_ctx(
      new rfl::scan::ASTScannerContext(package.get(), Basedir.getValue(),
                                       Verbose.getValue()));

  std::unique_ptr<rfl::scan::ASTScanActionFactory> factory(
      new rfl::scan::ASTScanActionFactory(scan_ctx.get()));

  int ret = tool.run(factory.get());

  if (Generators.empty()) {
    llvm::outs() << "No generator specified\n";
    return 1;
  }
  for (std::string const &generator : Generators) {
    llvm::outs() << "Using generator " << generator;
    std::string err;
    rfl::NativeLibrary lib = rfl::LoadNativeLibrary(generator.c_str(), &err);
    if (!lib) {
      llvm::errs() << err;
      continue;
    }
    CreateGenerator create_gen =
        (CreateGenerator)rfl::GetFunctionPointerFromNativeLibrary(
            lib, "CreateGenerator");
    if (create_gen != nullptr) {
      rfl::Generator *gen = create_gen();
      if (gen == nullptr) {
        llvm::errs() << "CreateGenerator returned null";
        continue;
      }
      gen->set_output_path(output_name.c_str());
      gen->set_generate_plugin(GeneratePlugin);
      gen->Generate(package.get());
      delete gen;
    } else {
      llvm::errs() << "Could not find symbol GeneratePackage";
      continue;
    }
  }
  return ret;
}
