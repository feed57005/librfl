#include "rfl-scan/ast_scan.h"
#include "rfl-scan/annotation_parser.h"
#include "rfl/reflected.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecordLayout.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"

#include <string>
#include <vector>

// TODO strip base path

namespace rfl {
namespace scan {

struct AnnoInserter {
  AnnoInserter(Annotation *anno) : anno_(anno) {}

  void operator()(std::string const &key, std::string const &value) const {
    anno_->AddEntry(key,value);
  }

  Annotation *anno_;
};

ASTScanner::ASTScanner(ASTScannerContext *scan_ctx, raw_ostream *out)
    : scanner_context_(scan_ctx), context_(nullptr), out_(out ? *out : outs()) {
}

void ASTScanner::HandleTranslationUnit(ASTContext &Context) {
  TranslationUnitDecl *D = Context.getTranslationUnitDecl();
  context_ = &Context;
  TraverseDecl(D);
}

bool ASTScanner::TraverseDecl(Decl *D) {
  NamedDecl *named_decl = cast<NamedDecl>(D);
  if (named_decl == nullptr || named_decl->isInvalidDecl())
    return base::TraverseDecl(D);

  // Filter out unsupported decls at the global namespace level
  switch (named_decl->getKind()) {
    case (Decl::CXXRecord):
      // skip forward declaration
      if (cast<CXXRecordDecl>(D)->isThisDeclarationADefinition() == VarDecl::DeclarationOnly)
        return base::TraverseDecl(D);
      else
        return _TraverseCXXRecord(cast<CXXRecordDecl>(D));

    case (Decl::Field):
      if (!isa<FieldDecl>(D))
        return base::TraverseDecl(D);
      return _TraverseFieldDecl(cast<FieldDecl>(D));

    case (Decl::CXXConstructor):
    case (Decl::CXXDestructor):
    case (Decl::CXXMethod):
    case (Decl::Function):
      if (isa<FunctionDecl>(D) && !cast<FunctionDecl>(D)->isFirstDecl())
        return base::TraverseDecl(D);
    case (Decl::Enum):
    case (Decl::ClassTemplate):
    case (Decl::ParmVar):
      AddDecl(named_decl);
      break;
    default:
      break;
  }
  return base::TraverseDecl(D);
}

static std::string StripBasedir(std::string const &filename,
                                std::string const &basedir) {
  size_t basedir_pos = filename.find(basedir);
  if (basedir_pos != std::string::npos && basedir_pos == 0) {
    return filename.substr(basedir.length() + 1,
                           filename.length() - basedir.length());
  }
  return filename;
}

static std::string PathRelativeToBaseDir(PresumedLoc const &presumed_loc,
                                         SourceManager const &src_manager,
                                         StringRef const &basedir) {
  StringRef hfile(presumed_loc.getFilename());
  SmallString<256> path(hfile.begin(), hfile.end());
  sys::fs::make_absolute(path);
  SmallVector<StringRef,16> components;
  SmallVector<StringRef, 16>::iterator comp_it = components.begin();
  for (sys::path::const_iterator path_it = sys::path::begin(path);
       path_it != sys::path::end(path); ++path_it) {
    StringRef component = *path_it;
    if (component.size() > 0 && component[0] == '.') {
        if (component.size() == 2 && comp_it > components.begin()){
          --comp_it;
        }
    } else {
      if (!component.empty())
        components.insert(comp_it++, component);
    }
  }
  components.resize(comp_it - components.begin());

  SmallString<256> full_path;
  SmallVector<StringRef, 16>::const_iterator it = components.begin();
  if (it != components.end()) {
    ++it;
  }
  for (; it != components.end(); ++it) {
    StringRef comp = *it;
    full_path.append(sys::path::get_separator());
    full_path.append(comp.begin(), comp.end());
  }

  StringRef path_str = full_path.str();
  if (path_str.size() > basedir.size() && path_str.startswith(basedir)) {
    path_str = path_str.substr(basedir.size()+1, path_str.size());
  }
  return path_str.str();
}

bool ASTScanner::ReadAnnotation(NamedDecl *D, Annotation *anno) {
  // Reflection attributes are stored as clang annotation attributes
  specific_attr_iterator<AnnotateAttr> i =
      D->specific_attr_begin<AnnotateAttr>();
  if (i == D->specific_attr_end<AnnotateAttr>()) {
    return false;
  }

  // Get the annotation text
  AnnotateAttr *attribute = *i;
  StringRef attribute_text = attribute->getAnnotation();

  SourceManager const &src_manager = context_->getSourceManager();
  SourceLocation location = attribute->getLocation();
  PresumedLoc presumed_loc = src_manager.getPresumedLoc(location);

  anno->value_ = attribute_text.str();
  anno->file_ = PathRelativeToBaseDir(presumed_loc, src_manager, StringRef(basedir()));
  anno->line_ = presumed_loc.getLine();

  std::string err_msg;
  std::unique_ptr<AnnotationParser> parser =
    AnnotationParser::loadFromBuffer(StringRef(anno->value_), err_msg);
  if (parser == nullptr) {
    errs() << "Failed to parse annotation: '" << anno->value_ << "'\n"
           << err_msg << "\n at " << anno->file_ << ":" << anno->line_ << "\n";
    return false;
  }
  parser->Enumerate(AnnoInserter(anno));
  return true;
}

Namespace *ASTScanner::GetOrCreateNamespaceForRecord(CXXRecordDecl *D) {
  DeclContext const *ctx = D->getDeclContext();

  typedef SmallVector<const DeclContext *, 8> ContextsTy;
  ContextsTy contexts;

  // Collect contexts.
  while (ctx && isa<NamedDecl>(ctx)) {
    contexts.push_back(ctx);
    ctx = ctx->getParent();
  }

  Namespace *current_ns = package();

  for (ContextsTy::reverse_iterator I = contexts.rbegin(), E = contexts.rend();
       I != E;
       ++I) {
    if (NamespaceDecl const *ND = dyn_cast<NamespaceDecl>(*I)) {
      std::string Result;
      llvm::raw_string_ostream OS(Result);
      OS << *ND;
      Namespace *ns = current_ns->FindNamespace(OS.str().c_str());
      if (!ns) {
        ns = new Namespace(OS.str());
        current_ns->AddNamespace(ns);
      }
      current_ns = ns;

    } else if (RecordDecl const *RD = dyn_cast<RecordDecl>(*I)) {
      out_ << "nested record not supported '" << *RD << "' ";
      return nullptr;
    } else {
      out_ << "?";
      return nullptr;
    }
  }

  return current_ns;
}

bool ASTScanner::_TraverseCXXRecord(CXXRecordDecl *D) {
  std::string qual_name = D->getQualifiedNameAsString();
  std::string name = D->getDeclName().getAsString();

  //out_ << "### " << qual_name.c_str() << "\n  " << *D << "\n";

  Annotation anno;
  if (ReadAnnotation(D, &anno)) {
    Namespace *ns = GetOrCreateNamespaceForRecord(D);
    Class *parent = CurrentClass();
    Class *super = nullptr;
    if (D->getNumBases()) {
      //out_ << name.c_str() << " base classes:\n";
      for (CXXBaseSpecifier const &base : D->bases()) {
        QualType qt = base.getType();
        Type const *type = qt.getTypePtrOrNull();
        if (type) {
          CXXRecordDecl *decl = type->getAsCXXRecordDecl();
          if (decl) {
            Namespace *base_ns = GetOrCreateNamespaceForRecord(decl);
            std::string Result;
            llvm::raw_string_ostream OS(Result);
            OS << *decl;
            super = base_ns->FindClass(OS.str().c_str());
            if (!super) {
              errs() << "Unable to find base class \n";
            }
          } else {
            errs() << "Unable to find base class \n";
            //return true;
          }
        }
      }
    }

    SourceManager const &src_manager = context_->getSourceManager();
    SourceLocation source_loc = D->getSourceRange().getBegin();
    PresumedLoc presumed_loc = src_manager.getPresumedLoc(source_loc, false);
    std::string header_file = StripBasedir(presumed_loc.getFilename(), basedir());

    Class *klass = new Class(name, header_file, anno, nullptr, nullptr, super);
    if (!parent) {
      ns->AddClass(klass);
    }
    class_queue_.push_front(klass);
    bool ret = base::TraverseDecl(D);
    class_queue_.pop_front();
    return ret;
  }
  return true;
}

bool ASTScanner::_TraverseFieldDecl(FieldDecl *D) {
  Annotation anno;
  if (ReadAnnotation(D, &anno)) {
    std::string type_name = D->getType().getAsString();
    std::string qual_name = D->getQualifiedNameAsString();
    std::string name = D->getDeclName().getAsString();
    Class *klass = CurrentClass();
    if (!klass) {
      errs() << "Error no enclosing class for " << name.c_str() << "\n";
      return true;
    }
    ASTRecordLayout const &layout = context_->getASTRecordLayout(D->getParent());
    uint32 offset = (uint32) layout.getFieldOffset(D->getFieldIndex());
    //uint64 size = context_->getASTRecordLayout(D).getSize().getQuantity();
    //out_ << klass->name().c_str() << " prop: " << name.c_str() << "\n";
    klass->AddProperty(new Property(name, type_name, offset, anno));
  }
  return true;
}

void ASTScanner::AddDecl(NamedDecl *D) {
  std::string name = D->getQualifiedNameAsString();
  Annotation anno;
  if (ReadAnnotation(D, &anno)) {
    out_ << D->getDeclKindName() << " " << name.c_str() << " : "
         << anno.value_.c_str() << " | " << anno.file_ << ":" << anno.line_
         << "\n";
  } else {
    //out_ << "-- " << D->getDeclKindName() << " " << name.c_str() << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////

ASTScanAction::ASTScanAction(ASTScannerContext *scan_ctx)
    : scanner_context_(scan_ctx) {
}

std::unique_ptr<ASTConsumer> ASTScanAction::CreateASTConsumer(
    CompilerInstance &CI,
    StringRef InFile) {
  return make_unique<ASTScanner>(scanner_context_, &outs());
}

} // namespace scan
}  // namespace rfl
