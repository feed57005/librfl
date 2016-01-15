// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "clang/Frontend/FrontendActions.h"
#include "clang/Driver/Options.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Basic/Version.h"

#include "llvm/Config/llvm-config.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/LineIterator.h"
#include "llvm/ADT/SmallString.h"

#include "google/protobuf/io/zero_copy_stream_impl.h"

#include "rfl/reflected.h"
#include "rfl/generator.h"
#include "rfl/native_library.h"
#include "rfl-scan/ast_scan.h"
#include "rfl-scan/compilation_db.h"
#include "rfl-scan/proto_ast_scan.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#if !defined(_LIBCPP_VERSION)
#error "libc++ not present"
#endif

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
                                    cl::desc("File with inputs"),
                                    cl::cat(RflScanCategory));

static cl::opt<bool> GeneratePlugin("plugin",
                                    cl::desc("Generate plugin (DEPRECATED)"),
                                    cl::cat(RflScanCategory));
static cl::opt<bool> GenerateProto("proto",
                                    cl::desc("Generate proto"),
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

static std::string NormalizedPath(std::string const &path) {
  std::string ret = path;
  if (!sys::path::is_separator(ret[ret.length()-1])) {
    ret += sys::path::get_separator();
  }
  return ret;
}

ArgumentsAdjuster GetInsertAdjuster(std::string const &extra) {
  return [extra] (CommandLineArguments const &args, StringRef /*unused*/){
    CommandLineArguments ret(args);
    ret.insert(ret.end(), extra);
    return ret;
  };
}

int ProtoScanner(ClangTool &tool) {
  using namespace std;
  using namespace rfl;
  using namespace rfl::scan;

  string basedir = NormalizedPath(Basedir.getValue());

  ScannerContext scan_ctx(basedir, Verbose.getValue());

  // setup package
  proto::Package &pkg = scan_ctx.package();
  pkg.set_name(PackageName.getValue());
  pkg.set_version(PackageVersion.getValue());

  // handle imports
  for (vector<string>::iterator it = Imports.begin(), e = Imports.end();
       it != e; ++it) {
    if (Verbose.getValue() > 1) {
      outs() << "Adding import " << *it << "\n";
    }
    pkg.add_imports(*it);
  }

  // handle libs
  for (vector<string>::iterator it = Libs.begin(), e = Libs.end(); it != e;
       ++it) {
    if (Verbose.getValue() > 1) {
      outs() << "Adding library " << *it << "\n";
    }
    pkg.add_libraries(*it);
  }

  outs().flush();

  unique_ptr<ScannerActionFactory> factory(new ScannerActionFactory(&scan_ctx));

  int ret = tool.run(factory.get());
  if (ret == 0) {
    string file = OutputFile.getValue();

    // Make sure that output directory exists
    //SmallString<256> path(file);
    //sys::path::remove_filename(path);
    //if (sys::fs::create_directories(path, true)) {
    //  outs() << "Unable to create directory\n";
    //  return false;
    //}

    if (Verbose.getValue() > 1) {
      outs() << "Writing proto file " << file << "\n";
      outs().flush();
    }

    // Prepare output file
    // TODO Write to temporary file. If output file already exists
    // check for modifications and if files differ, replace it.
    ofstream file_out;
    file_out.open(file, ios_base::binary);
    if (!file_out.good()) {
      errs() << "Failed to open file " << file << " " << strerror(errno)
             << "\n";
      file_out.close();
      return 1;
    }

    // write header
    char magic[5] = {'R', 'F', 'L', 1, 0};
    file_out << magic;

    // write protobuf to file
    {
      google::protobuf::io::OstreamOutputStream proto_out(&file_out);
      if (!pkg.SerializeToZeroCopyStream(&proto_out)) {
        errs() << "Failed to write file " << file << " " << strerror(errno)
               << "\n";
        file_out.close();
        return 1;
      }
    }

    file_out.flush();
    file_out.close();
  } else {
    errs() << "Scanning failed " << ret << "\n";
    errs().flush();
  }

  return ret;
}

int LegacyScanner(ClangTool &tool,
                  std::vector<std::string> const &source_path_list) {
  using namespace std;
  using namespace rfl;

  string output_path = NormalizedPath(OutputPath.getValue());
  string basedir = NormalizedPath(Basedir.getValue());
  string const &pkg_name = PackageName.getValue();
  string const &pkg_version = PackageVersion.getValue();
  if (Verbose.getValue() > 1) {
    outs() << "output path: " << output_path << "\n"
           << "basedir: " << basedir << "\n";
    outs().flush();
  }

  unique_ptr<Package> package(
      new Package(pkg_name.c_str(), pkg_version.c_str()));

  // handle imports
  for (vector<string>::iterator it = Imports.begin(), e = Imports.end();
       it != e; ++it) {
    if (Verbose.getValue() > 1) {
      outs() << "importing " << *it << "\n";
    }
    package->AddImport((*it).c_str());
  }
  outs().flush();

  // handle libs
  for (vector<string>::iterator it = Libs.begin(), e = Libs.end(); it != e;
       ++it) {
    package->AddLibrary((*it).c_str());
  }

  // handle sources
  for (string const &src : source_path_list) {
    string rel_src = StripBasedir(src, basedir);
    PackageFile *pkg_file = package->GetOrCreatePackageFile(rel_src.c_str());
    pkg_file->set_is_dependecy(false);
  }

  unique_ptr<scan::ASTScannerContext> scan_ctx(
      new scan::ASTScannerContext(package.get(), basedir, Verbose.getValue()));

  unique_ptr<scan::ASTScanActionFactory> factory(
      new scan::ASTScanActionFactory(scan_ctx.get()));

  int ret = tool.run(factory.get());
  if (ret != 0) {
    errs() << "Scanning failed " << ret << "\n";
    errs().flush();
    return ret;
  }

  if (Generators.empty()) {
    outs() << "No generator specified\n";
    return 1;
  }

  for (string const &generator : Generators) {
    if (Verbose.getValue() > 1) {
      outs() << "Using generator " << generator << "\n";
      outs().flush();
    }

    string err;
    NativeLibrary lib = LoadNativeLibrary(generator.c_str(), &err);
    if (!lib) {
      errs() << "Failed to load native library '" << generator
             << "' : " << err << "\n";
      errs().flush();
      ret = 1;
      continue;
    }

    typedef Generator *(*CreateGenerator)();
    CreateGenerator create_gen =
        (CreateGenerator)GetFunctionPointerFromNativeLibrary(lib,
                                                             "CreateGenerator");
    if (create_gen != nullptr) {
      Generator *gen = create_gen();
      if (gen == nullptr) {
        errs() << "CreateGenerator() returned null\n";
        errs().flush();
        ret = 1;
        continue;
      }
      gen->set_output_path(output_path.c_str());
      gen->set_output_file(OutputFile.getValue().c_str());
      gen->set_generate_plugin(GeneratePlugin);
      gen->Generate(package.get());
      delete gen;
    } else {
      errs() << "Could not find symbol 'CreateGenerator'\n";
      errs().flush();
      ret = 1;
      continue;
    }
  }
  return ret;
}

int main(int argc, char const **argv) {
  sys::PrintStackTraceOnErrorSignal();

  CommonOptionsParser options_parser(argc, argv, RflScanCategory);

  // Fill source paths from input file or command line
  std::vector<std::string> source_path_list;
  if (InputFile.getNumOccurrences() == 0) {
    source_path_list = options_parser.getSourcePathList();
  } else {
    ErrorOr<std::unique_ptr<MemoryBuffer>> input_buffer =
        MemoryBuffer::getFile(InputFile.getValue());
    if (std::error_code result = input_buffer.getError()) {
      errs() << "Error while opening JSON database: " + result.message();
      errs().flush();
      return 1;
    }

    line_iterator line_it(*input_buffer.get());
    while (!line_it.is_at_end()) {
      source_path_list.push_back(*line_it);
      ++line_it;
    }
  }

  // Create compile flags DB
  CompilationDatabase &default_cdb = options_parser.getCompilations();
  rfl::scan::ScanCompilationDatabase cdb(default_cdb, source_path_list);

  // Determine path to clang includes
  // TODO compilation define for external/local clang headers
  SmallString<128> resource_dir(LLVM_PREFIX);
  sys::path::append(resource_dir, "lib", "clang", CLANG_VERSION_STRING);

  // Set internal / extra compilation flags
  CommandLineArguments extra_args;
  extra_args.push_back("-resource-dir");
  extra_args.push_back(resource_dir.str());
  extra_args.push_back("-D__RFL_SCAN__");
  extra_args.push_back("-Wno-unused-local-typedef");  // TODO hack

  // Split and insert includes from IMPLICIT define
  std::istringstream iss(std::string(IMPLICIT));
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter(extra_args));

  if (Verbose.getValue() > 2) {
    outs() << "Extra arguments:";
    for (std::string const &arg : extra_args) {
      outs() << " " << arg;
    }
    outs() << "\n";
    outs().flush();
  }

  // Create ClangTool
  ClangTool tool(cdb, source_path_list);
  tool.clearArgumentsAdjusters();
  tool.appendArgumentsAdjuster(
      getInsertArgumentAdjuster(extra_args, ArgumentInsertPosition::BEGIN));

  if (GenerateProto.getValue()) {
    return ProtoScanner(tool);
  } else {
    return LegacyScanner(tool, source_path_list);
  }
}
