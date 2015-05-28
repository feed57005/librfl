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

// TODO document & externalize mandatory annotation tokens

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

    case (Decl::CXXMethod):
      if (isa<CXXMethodDecl>(D) && cast<CXXMethodDecl>(D)->isFirstDecl())
        return _TraverseMethodDecl(cast<CXXMethodDecl>(D));
      return base::TraverseDecl(D);

    case (Decl::CXXConstructor):
    case (Decl::CXXDestructor):
    case (Decl::Function):
      if (isa<FunctionDecl>(D) && !cast<FunctionDecl>(D)->isFirstDecl())
        return base::TraverseDecl(D);
      break;
    case (Decl::Enum):
      if (isa<EnumDecl>(D) &&
          cast<EnumDecl>(D)->isThisDeclarationADefinition() !=
              VarDecl::DeclarationOnly)
        return _TraverseEnumDecl(cast<EnumDecl>(D));
      break;
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

  anno->file_ = PathRelativeToBaseDir(presumed_loc, src_manager, StringRef(basedir()));
  anno->line_ = presumed_loc.getLine();

  std::string err_msg;
  std::unique_ptr<AnnotationParser> parser =
    AnnotationParser::loadFromBuffer(attribute_text, err_msg);
  if (parser == nullptr) {
    errs() << "Failed to parse annotation: '" << anno->value_ << "'\n"
           << err_msg << "\n at " << anno->file_ << ":" << anno->line_ << "\n";
    return false;
  }
  anno->value_ = parser->kind();
  parser->Enumerate(AnnoInserter(anno));
  return true;
}

Namespace *ASTScanner::GetOrCreateNamespaceForRecord(Decl *D) {
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
      errs() << "nested record not supported '" << *RD << "' \n";
      return nullptr;
    } else {
      errs() << "Unknown declaration context\n";
      return nullptr;
    }
  }

  return current_ns;
}

bool ASTScanner::_TraverseCXXRecord(CXXRecordDecl *D) {
  std::string qual_name = D->getQualifiedNameAsString();

  std::string name = D->getDeclName().getAsString();

  Annotation anno;
  if (ReadAnnotation(D, &anno)) {
    Namespace *ns = GetOrCreateNamespaceForRecord(D);
    if (ns->FindClass(name.c_str())) {
      // already processed
      return true;
    }
    Class *parent = CurrentClass();
    Class *super = nullptr;
    if (D->getNumBases()) {
      for (CXXBaseSpecifier const &base : D->bases()) {
        QualType qt = base.getType();
        Type const *type = qt.getTypePtrOrNull();
        if (type) {
          CXXRecordDecl *decl = type->getAsCXXRecordDecl();
          if (decl) {
            Namespace *base_ns = GetOrCreateNamespaceForRecord(decl);
            std::string result;
            llvm::raw_string_ostream OS(result);
            OS << *decl;
            super = base_ns->FindClass(OS.str().c_str());
            if (super)
              break;
          } else {
            errs() << "Unable to find base class " << qual_name << " among bases\n";
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
    std::string type_name = D->getType().getLocalUnqualifiedType().getAsString();
    PrintingPolicy policy(context_->getLangOpts());
    policy.SuppressTagKeyword = true;
    policy.SuppressScope = true;
    type_name = D->getType().getAsString(policy);
    std::string name = D->getDeclName().getAsString();
    Class *klass = CurrentClass();
    if (!klass) {
      errs() << "Error no enclosing class for " << name.c_str() << "\n";
      return true;
    }
    ASTRecordLayout const &layout = context_->getASTRecordLayout(D->getParent());
    uint32 offset = (uint32) layout.getFieldOffset(D->getFieldIndex());
    klass->AddProperty(new Property(name, type_name, offset, anno));
  }
  return true;
}

bool ASTScanner::_TraverseEnumDecl(EnumDecl *D) {
  Annotation anno;

  std::string name = D->getQualifiedNameAsString();
  name = D->getName().str();
  if (!ReadAnnotation(D, &anno)) {
    return true;
  }
  SourceManager const &src_manager = context_->getSourceManager();
  SourceLocation source_loc = D->getSourceRange().getBegin();
  PresumedLoc presumed_loc = src_manager.getPresumedLoc(source_loc, false);
  std::string header_file = StripBasedir(presumed_loc.getFilename(), basedir());
  Class *parent = CurrentClass();
  Namespace *ns = nullptr;
  if (parent == nullptr) {
    ns = GetOrCreateNamespaceForRecord(D);
  }
  Enum *e = new Enum(name, header_file, anno, ns, parent);
  for (EnumDecl::enumerator_iterator it = D->enumerator_begin();
       it != D->enumerator_end(); ++it) {
    EnumConstantDecl *e_item = *it;
    Enum::EnumItem item;
    item.id_ = e_item->getNameAsString();
    llvm::APSInt const &value = e_item->getInitVal();
    item.value_ = (long) value.getSExtValue();
    item.name_ = anno.GetEntry(item.id_); // XXX check
    e->AddEnumItem(item);
  }
  if (!parent) {
    ns->AddEnum(e);
  } else {
    parent->AddEnum(e);
  }
  return true;
}

bool ASTScanner::_TraverseMethodDecl(CXXMethodDecl *D) {
  // Ignore overloaded operators for now
  if (D->isOverloadedOperator() || class_queue_.empty())
    return true;
  Annotation anno;
  if (!ReadAnnotation(D, &anno))
    return true;
  std::string name = D->getDeclName().getAsString();
  PrintingPolicy policy(context_->getLangOpts());
  policy.SuppressTagKeyword = true;
  policy.SuppressScope = true;
  std::string ret_type =
      D->getReturnType().getLocalUnqualifiedType().getAsString();

  Class *klass = class_queue_.front();
  Method *method = new Method(name, anno);
  klass->AddMethod(method);
  method->AddArgument(
      new Argument("return", Argument::kReturn_Kind, ret_type, Annotation()));

  for (CXXMethodDecl::param_iterator it = D->param_begin();
       it != D->param_end(); ++it) {
    ParmVarDecl *param = *it;
    Annotation param_anno;
    if (!ReadAnnotation(param, &param_anno)) {
      errs() << "parameter not annotated\n";
      continue;
    }
    llvm::StringRef param_name = param->getName().str();
    if (param_name.empty()) {
      errs() << "Unnamed parameter\n";
      continue;
    }
    std::string param_type =
        param->getType().getLocalUnqualifiedType().getAsString();
    char const *kind_entry = param_anno.GetEntry("kind");
    if (!kind_entry) {
      errs() << "Missing 'kind' annotation on argument " << param_name << "\n";
      // FIXME method & params are invalid, release memory
      return true;
    }
    std::string kind_anno = kind_entry;
    Argument::Kind kind;
    if (kind_anno.compare("in") == 0) {
      kind = Argument::kInput_Kind;
    } else if (kind_anno.compare("out") == 0) {
      kind = Argument::kOutput_Kind;
    } else if (kind_anno.compare("inout") == 0) {
      kind = Argument::kInOut_Kind;
    } else {
      errs() << "Unknown argument kind '" << kind_anno << "'\n";
      // FIXME method & params are invalid, release memory
      return true;
    }

    Argument *arg = new Argument(param_name, kind, param_type, param_anno);
    method->AddArgument(arg);
  }

  return true;
}

void ASTScanner::AddDecl(NamedDecl *D) {
  std::string name = D->getQualifiedNameAsString();
  Annotation anno;
  if (ReadAnnotation(D, &anno)) {
    outs() << "Found unknown annotated declaration " << D->getDeclKindName()
           << " " << name.c_str() << " : " << anno.value_.c_str() << " | "
           << anno.file_ << ":" << anno.line_ << "\n";
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
