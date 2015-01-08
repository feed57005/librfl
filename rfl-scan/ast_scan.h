#ifndef __RFL_AST_SCAN_ACTION_H__
#define __RFL_AST_SCAN_ACTION_H__

#include "rfl/reflected.h"

#include "clang/Frontend/FrontendAction.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Tooling/Tooling.h"

#include <deque>

namespace rfl {
namespace scan {
  
using namespace llvm;
using namespace clang;

class ASTScanner : public ASTConsumer, public RecursiveASTVisitor<ASTScanner> {
  typedef RecursiveASTVisitor<ASTScanner> base;

public:
  ASTScanner(Package *pkg, raw_ostream *out = nullptr);

  void HandleTranslationUnit(ASTContext &Context) override;
  bool shouldWalkTypesOfTypeLocs() const { return false; }

  bool TraverseDecl(Decl *D);

private:
  bool ReadAnnotation(NamedDecl *D, Annotation *anno);
  Class *CurrentClass() const {
    return !class_queue_.empty() ? class_queue_.front() : nullptr;
  }
  bool _TraverseCXXRecord(CXXRecordDecl *D);

  bool _TraverseFieldDecl(FieldDecl *D);

  void AddDecl(NamedDecl *D);

  Namespace *GetOrCreateNamespaceForRecord(CXXRecordDecl *D);

private:
  ASTContext *context_;
  raw_ostream &out_;
  Package *package_;
  std::deque<Namespace *> namespace_queue_;
  std::deque<Class *> class_queue_;
};

class ASTScanAction : public ASTFrontendAction {
public:
  ASTScanAction(Package *package);
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;
private:
  Package *package_;
};

class ASTScanActionFactory : public tooling::FrontendActionFactory {
public:
  ASTScanActionFactory(Package *pkg) : package_(pkg) {}

  clang::FrontendAction *create() override {
    return new ASTScanAction(package_);
  }

private:
  Package *package_;
};

} // namespace scan
} // namespace rfl

#endif /* __RFL_AST_SCAN_ACTION_H__ */
