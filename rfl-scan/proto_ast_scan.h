// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __RFL_SCAN_PROTO_AST_SCAN_H__
#define __RFL_SCAN_PROTO_AST_SCAN_H__

#include "rfl/reflected.pb.h"

#include "clang/Frontend/FrontendAction.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Tooling/Tooling.h"

#include <deque>

namespace rfl {

class Annotation;

namespace scan {

using namespace llvm;
using namespace clang;

class ScannerContext {
public:
  ScannerContext(std::string const &basedir, unsigned verbose = 0)
      : basedir_(basedir), verbose_(verbose), class_count_(0) {}

  proto::Package const &package() const { return package_; }
  proto::Package &package() { return package_; }

  unsigned verbose() const { return verbose_; }
  std::string const &basedir() const { return basedir_; }

  unsigned class_count() const { return class_count_; }
  void set_class_count(unsigned count) { class_count_ = count; }

private:
  proto::Package package_;
  std::string basedir_;
  unsigned verbose_;
  unsigned class_count_;
};

class Scanner : public ASTConsumer, public RecursiveASTVisitor<Scanner> {
  typedef RecursiveASTVisitor<Scanner> Base;

public:
  Scanner(ScannerContext *scan_ctx, raw_ostream *out = nullptr);

  // ASTConsumer overrides
  void HandleTranslationUnit(ASTContext &Context) override;

  // RecursiveASTVisitor template overrides
  bool shouldWalkTypesOfTypeLocs() const { return false; }

  bool TraverseCXXRecordDecl(CXXRecordDecl *D);
  bool VisitCXXRecordDecl(CXXRecordDecl *D);
  bool VisitNamespaceDecl(NamespaceDecl *D);
  bool TraverseNamespaceDecl(NamespaceDecl *D);

  bool VisitFunctionDecl(FunctionDecl *D);
  bool VisitFieldDecl(FieldDecl *D);
  bool VisitCXXMethodDecl(CXXMethodDecl *D);
  bool VisitEnumDecl(EnumDecl *D);
  bool VisitTypedefDecl(TypedefDecl *D);

private:
  bool IsCurrentFileLocation(SourceLocation loc) const;
  bool HasAnnotation(NamedDecl *D) const;

  bool ReadAnnotation(NamedDecl *D, proto::Annotation *anno);
  void LogDecl(NamedDecl *D) const;

  bool ReadType(QualType qt, proto::TypeRef *tr, proto::TypeQualifier *tq);

  proto::Package &package() const { return scanner_context_->package(); }
  std::string const &basedir() const { return scanner_context_->basedir(); }
  unsigned verbose() const { return scanner_context_->verbose(); }
  SourceManager const &src_manager() const;

  unsigned class_count() const { return scanner_context_->class_count(); }
  void set_class_count(unsigned count) {
    scanner_context_->set_class_count(count);
  }

  proto::Class *CurrentClass() const {
    return !class_queue_.empty() ? class_queue_.front() : nullptr;
  }
  proto::Namespace *CurrentNamespace() const {
    return !namespace_queue_.empty() ? namespace_queue_.front() : nullptr;
  }

private:
  ScannerContext *scanner_context_;
  ASTContext *context_;
  raw_ostream &out_;
  proto::PackageFile *current_file_;
  std::deque<proto::Namespace *> namespace_queue_;
  std::deque<proto::Class *> class_queue_;
  SourceLocation current_file_location_;
};

////////////////////////////////////////////////////////////////////////////////

class ScannerAction : public ASTFrontendAction {
public:
  ScannerAction(ScannerContext *scan_ctx) : scanner_context_(scan_ctx) {}

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    return make_unique<Scanner>(scanner_context_, &outs());
  }

private:
  ScannerContext *scanner_context_;
};

class ScannerActionFactory : public tooling::FrontendActionFactory {
public:
  ScannerActionFactory(ScannerContext *scan_ctx) : scanner_context_(scan_ctx) {}

  clang::FrontendAction *create() override {
    return new ScannerAction(scanner_context_);
  }

private:
  ScannerContext *scanner_context_;
};

} // namespace scan
} // namespace rfl

#endif /* __RFL_SCAN_PROTO_AST_SCAN_H__ */
