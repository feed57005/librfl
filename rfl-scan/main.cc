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
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/LineIterator.h"
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

static cl::opt<std::string> OutputPath("output-dir",
                                       cl::desc("Output directory"),
                                       cl::cat(RflScanCategory));
static cl::opt<std::string> OutputFile("o",
                                       cl::desc("Output file with list of generated files"),
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
static cl::opt<std::string> InputFile("input",
                                    cl::desc("file with inputs"),
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
    return filename.substr(basedir.length(),
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

  std::vector<std::string> source_path_list;
  if (InputFile.getNumOccurrences() == 0) {
    source_path_list =
        options_parser.getSourcePathList();
  } else {
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> input_buffer =
        llvm::MemoryBuffer::getFile(InputFile.getValue());
    if (std::error_code result = input_buffer.getError()) {
      llvm::errs() << "Error while opening JSON database: " + result.message();
      return 1;
    }
    llvm::line_iterator line_it(*input_buffer.get());
    while (!line_it.is_at_end()) {
      source_path_list.push_back(*line_it);
      ++line_it;
    }
  }
  CompilationDatabase &default_cdb = options_parser.getCompilations();
  rfl::scan::ScanCompilationDatabase cdb(default_cdb, source_path_list);
  ClangTool tool(cdb, source_path_list);
  tool.clearArgumentsAdjusters();

  SmallString<128> resource_dir(LLVM_PREFIX);
  llvm::sys::path::append(resource_dir , "lib", "clang", CLANG_VERSION_STRING);

  CommandLineArguments extra_args;

  extra_args.push_back("-resource-dir");
  extra_args.push_back(resource_dir.str());
  extra_args.push_back("-D__RFL_SCAN__");
  extra_args.push_back("-Wno-unused-local-typedef"); // TODO hack

  // split and insert IMPLICIT includes
  std::istringstream iss(std::string(IMPLICIT));
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter(extra_args));

  if (Verbose.getValue() > 1) {
    for (std::string const &arg : extra_args) {
      outs() << arg << "\n";
    }
    outs() << "using resource dir: " << resource_dir.c_str() << "\n";
    outs().flush();
  }

  tool.appendArgumentsAdjuster(
     getInsertArgumentAdjuster(extra_args, ArgumentInsertPosition::BEGIN));

  std::string output_path = OutputPath.getValue();
  if (!llvm::sys::path::is_separator(output_path[output_path.length()-1])) {
    output_path += llvm::sys::path::get_separator();
  }
  std::string basedir = Basedir.getValue();
  if (!llvm::sys::path::is_separator(basedir[basedir.length()-1])) {
    basedir += llvm::sys::path::get_separator();
  }
  std::string const &pkg_name = PackageName.getValue();
  std::string const &pkg_version = PackageVersion.getValue();
  if (Verbose.getValue() > 1) {
    llvm::outs() << "output path: " << output_path << "\n"
      << "basedir: " << basedir << "\n";
    llvm::outs().flush();
  }
  std::unique_ptr<rfl::Package> package(new rfl::Package(pkg_name.c_str(), pkg_version.c_str()));

  // handle imports
  for (std::vector<std::string>::iterator it = Imports.begin(),
                                          e = Imports.end();
       it != e; ++it) {
    if (Verbose.getValue() > 1) {
      llvm::outs() << "importing " << *it << "\n";
    }
    package->AddImport((*it).c_str());
  }
  llvm::outs().flush();

  // handle libs
  for (std::vector<std::string>::iterator it = Libs.begin(), e = Libs.end();
       it != e; ++it) {
    package->AddLibrary((*it).c_str());
  }

  // handle sources
  for (std::string const &src : source_path_list) {
    std::string rel_src = StripBasedir(src, basedir);
    rfl::PackageFile *pkg_file =
        package->GetOrCreatePackageFile(rel_src.c_str());
    pkg_file->set_is_dependecy(false);
  }

  std::unique_ptr<rfl::scan::ASTScannerContext> scan_ctx(
      new rfl::scan::ASTScannerContext(package.get(), basedir,
                                       Verbose.getValue()));

  std::unique_ptr<rfl::scan::ASTScanActionFactory> factory(
      new rfl::scan::ASTScanActionFactory(scan_ctx.get()));

  int ret = tool.run(factory.get());

  if (Generators.empty()) {
    llvm::outs() << "No generator specified\n";
    return 1;
  }
  for (std::string const &generator : Generators) {
    llvm::outs() << "Using generator " << generator << "\n";
    llvm::outs().flush();
    std::string err;
    rfl::NativeLibrary lib = rfl::LoadNativeLibrary(generator.c_str(), &err);
    if (!lib) {
      llvm::errs() << err << "\n";
      llvm::outs().flush();
      ret = 1;
      continue;
    }
    CreateGenerator create_gen =
        (CreateGenerator)rfl::GetFunctionPointerFromNativeLibrary(
            lib, "CreateGenerator");
    if (create_gen != nullptr) {
      rfl::Generator *gen = create_gen();
      if (gen == nullptr) {
        llvm::errs() << "CreateGenerator() returned null\n";
        llvm::errs().flush();
        ret = 1;
        continue;
      }
      gen->set_output_path(output_path.c_str());
      gen->set_output_file(OutputFile.getValue().c_str());
      gen->set_generate_plugin(GeneratePlugin);
      gen->Generate(package.get());
      delete gen;
    } else {
      llvm::errs() << "Could not find symbol 'CreateGenerator'\n";
      llvm::errs().flush();
      ret = 1;
      continue;
    }
  }
  return ret;
}
