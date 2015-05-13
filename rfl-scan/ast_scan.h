#ifndef __RFL_AST_SCAN_ACTION_H__
#define __RFL_AST_SCAN_ACTION_H__

#include "rfl/reflected.h"
#include "rfl/repository.h"

#include "clang/Frontend/FrontendAction.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Tooling/Tooling.h"

#include <deque>

namespace rfl {
namespace scan {

using namespace llvm;
using namespace clang;

class ASTScannerContext {
public:
  ASTScannerContext(Repository *repository, Package *pkg, std::string const &basedir)
    : repository_(repository), package_(pkg), basedir_(basedir) {}

  Repository *repository() const { return repository_; }
  Package *package() const { return package_; }
  std::string const &basedir() const { return basedir_; }

private:
  Repository *repository_;
  Package *package_;
  std::string basedir_;
};

class ASTScanner : public ASTConsumer, public RecursiveASTVisitor<ASTScanner> {
  typedef RecursiveASTVisitor<ASTScanner> base;

public:
  ASTScanner(ASTScannerContext *scan_ctx, raw_ostream *out = nullptr);

  void HandleTranslationUnit(ASTContext &Context) override;
  bool shouldWalkTypesOfTypeLocs() const { return false; }

  bool TraverseDecl(Decl *D);

private:
  bool ReadAnnotation(NamedDecl *D, Annotation *anno);
  Class *CurrentClass() const {
    return !class_queue_.empty() ? class_queue_.front() : nullptr;
  }
  bool _TraverseCXXRecord(CXXRecordDecl *D);

  bool _TraverseEnumDecl(EnumDecl *D);

  bool _TraverseFieldDecl(FieldDecl *D);

  bool _TraverseMethodDecl(CXXMethodDecl *D);

  void AddDecl(NamedDecl *D);

  Namespace *GetOrCreateNamespaceForRecord(Decl *D);

  Package *package() const { return scanner_context_->package(); }
  std::string const &basedir() const { return scanner_context_->basedir(); }

private:
  ASTScannerContext *scanner_context_;
  ASTContext *context_;
  raw_ostream &out_;
  std::deque<Namespace *> namespace_queue_;
  std::deque<Class *> class_queue_;
};

class ASTScanAction : public ASTFrontendAction {
public:
  ASTScanAction(ASTScannerContext *scan_ctx);
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;
private:
  ASTScannerContext *scanner_context_;
};

class ASTScanActionFactory : public tooling::FrontendActionFactory {
public:
  ASTScanActionFactory(ASTScannerContext *scan_ctx) : scanner_context_(scan_ctx) {}

  clang::FrontendAction *create() override {
    return new ASTScanAction(scanner_context_);
  }

private:
  ASTScannerContext *scanner_context_;
};

} // namespace scan
} // namespace rfl

#endif /* __RFL_AST_SCAN_ACTION_H__ */
