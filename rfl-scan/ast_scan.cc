// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rfl-scan/ast_scan.h"
#include "rfl-scan/annotation_parser.h"
#include "rfl/reflected.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecordLayout.h"
#include "clang/AST/TypeLoc.h"
#include "clang/AST/Type.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/TargetInfo.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ErrorHandling.h"

#include <string>
#include <vector>

// TODO document & externalize mandatory annotation tokens

namespace rfl {
namespace scan {

namespace {

struct AnnoInserter {
  AnnoInserter(Annotation *anno) : anno_(anno) {}

  void operator()(std::string const &key, std::string const &value) const {
    anno_->AddEntry(key.c_str(), value.c_str());
  }

  Annotation *anno_;
};

struct AnnoDebugPrinter {
  void operator()(std::string const &key, std::string const &value) const {
    outs() << "  " << key << " : " << value << "\n";
  }
};

static std::string PathRelativeToBaseDir(SourceLocation const &source_loc,
                                         SourceManager const &src_manager,
                                         StringRef const &basedir) {

  PresumedLoc presumed_loc = src_manager.getPresumedLoc(source_loc, false);
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
    path_str = path_str.substr(basedir.size(), path_str.size());
  }
  return path_str.str();
}

} // namespace

////////////////////////////////////////////////////////////////////////////////

ASTScanner::ASTScanner(ASTScannerContext *scan_ctx, raw_ostream *out)
    : scanner_context_(scan_ctx), context_(nullptr), out_(out ? *out : outs()) {
}

void ASTScanner::HandleTranslationUnit(ASTContext &Context) {
  TranslationUnitDecl *D = Context.getTranslationUnitDecl();
  context_ = &Context;

  if (verbose()) {
    FileID main_id = src_manager().getMainFileID();
    SourceLocation location = src_manager().getLocForStartOfFile(main_id);
    outs() << "Translation unit: " << this << " ";
    location.print(outs(), src_manager());
    outs() << "\n";
  }
  TraverseDecl(D);
}

bool ASTScanner::TraverseDecl(Decl *D) {
  NamedDecl *named_decl = cast<NamedDecl>(D);
  if (named_decl == nullptr || named_decl->isInvalidDecl())
    return Base::TraverseDecl(D);

  // Filter out unsupported decls at the global namespace level
  switch (named_decl->getKind()) {
    case (Decl::CXXRecord):
      // skip forward declaration
      if (cast<CXXRecordDecl>(D)->isThisDeclarationADefinition() == VarDecl::DeclarationOnly)
        return Base::TraverseDecl(D);
      else
        return _TraverseCXXRecord(cast<CXXRecordDecl>(D));

    case (Decl::Field):
      if (!isa<FieldDecl>(D))
        return Base::TraverseDecl(D);
      return _TraverseFieldDecl(cast<FieldDecl>(D));

    case (Decl::CXXMethod):
      if (isa<CXXMethodDecl>(D) && cast<CXXMethodDecl>(D)->isFirstDecl())
        return _TraverseMethodDecl(cast<CXXMethodDecl>(D));
      return Base::TraverseDecl(D);

    case (Decl::CXXConstructor):
    case (Decl::CXXDestructor):
    case (Decl::Function):
      if (isa<FunctionDecl>(D) && !cast<FunctionDecl>(D)->isFirstDecl())
        return Base::TraverseDecl(D);
      break;
    case (Decl::Enum):
      if (isa<EnumDecl>(D) &&
          cast<EnumDecl>(D)->isThisDeclarationADefinition() !=
              VarDecl::DeclarationOnly)
        return _TraverseEnumDecl(cast<EnumDecl>(D));
      break;
    case (Decl::Typedef):
      if (isa<TypedefDecl>(D))
        return _TraverseTypedefDecl(cast<TypedefDecl>(D));
      break;
    case (Decl::ClassTemplate):
      AddDecl(named_decl);
      break;
    default:
      break;
  }
  return Base::TraverseDecl(D);
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

  SourceLocation location = attribute->getLocation();
  if (verbose() > 2) {
    location.print(outs(), src_manager());
    outs() << " | annotation: '" << attribute_text << "'\n";
    outs().flush();
  }

  std::string err_msg;
  std::unique_ptr<AnnotationParser> parser =
    AnnotationParser::loadFromBuffer(attribute_text, err_msg);
  if (parser == nullptr) {
    location.print(errs(), src_manager());
    errs() << " Failed to parse annotation: '" << attribute_text << "'\n";
    return false;
  }

  if (anno != nullptr) {
    anno->set_kind(parser->kind());
    parser->Enumerate(AnnoInserter(anno));
  }

  if (verbose() > 2) {
    parser->Enumerate(AnnoDebugPrinter());
  }
  return true;
}

Namespace *ASTScanner::GetOrCreateNamespaceForRecord(Decl *D) {
  DeclContext const *ctx = D->getDeclContext();

  typedef SmallVector<const DeclContext *, 8> ContextsTy;
  ContextsTy contexts;

  // collect contexts.
  while (ctx && isa<NamedDecl>(ctx)) {
    contexts.push_back(ctx);
    ctx = ctx->getParent();
  }

  Namespace *current_ns = package();

  for (ContextsTy::reverse_iterator I = contexts.rbegin(), E = contexts.rend();
       I != E;
       ++I) {
    if (NamespaceDecl const *ND = dyn_cast<NamespaceDecl>(*I)) {
      std::string ns_name;
      llvm::raw_string_ostream os(ns_name);
      os << *ND;
      os.flush();
      Namespace *ns = current_ns->FindNamespace(ns_name.c_str());
      if (!ns) {
        ns = new Namespace(os.str().c_str());
        current_ns->AddNamespace(ns);
      }
      current_ns = ns;

    } else if (RecordDecl const *RD = dyn_cast<RecordDecl>(*I)) {
      errs() << "nested record not supported '" << *RD << "' \n";
      errs().flush();
      return nullptr;
    } else {
      errs() << "Unknown declaration context\n";
      return nullptr;
    }
  }

  return current_ns;
}

bool ASTScanner::_TraverseTypedefDecl(TypedefDecl *D) {
  Annotation anno;
  if (!ReadAnnotation(D, &anno)) {
    return true;
  }

  Namespace *ns = GetOrCreateNamespaceForRecord(D);
  if (!ns)
    return true;

  std::string name = D->getDeclName().getAsString();

  if (ns->FindClass(name.c_str())) {
    // already processed
    return true;
  }

  LogDecl(D);

  SourceLocation source_loc = D->getSourceRange().getBegin();
  std::string header_file =
      PathRelativeToBaseDir(source_loc, src_manager(), basedir());
  PackageFile *pkg_file =
      package()->GetOrCreatePackageFile(header_file.c_str());

  Class *klass = new Class(name.c_str(), pkg_file, anno);
  klass->set_order(class_count());
  set_class_count(class_count() + 1);
  ns->AddClass(klass);

  return true;
}

bool ASTScanner::_TraverseCXXRecord(CXXRecordDecl *D) {
  Annotation anno;
  if (!ReadAnnotation(D, &anno)) {
    return true;
  }

  LogDecl(D);

  std::string qual_name = D->getQualifiedNameAsString();
  std::string name = D->getDeclName().getAsString();

  Class *parent = CurrentClass();
  Class *super = nullptr;
  Namespace *ns = nullptr;
  // check that class does not already exists
  if (parent) {
    // this class decl is nested, look in parent class
    if (parent->FindClass(name.c_str()))
      return true;
  } else {
    // this class decl is nested, look in parent class
    ns = GetOrCreateNamespaceForRecord(D);
    if (!ns || ns->FindClass(name.c_str())) {
      // already processed
      return true;
    }
  }

  uint32 base_class_offset = 0;

  if (D->getNumBases()) {
    bool has_unannotated_base_class = false;
    for (CXXBaseSpecifier const &base : D->bases()) {
      QualType qt = base.getType();

      clang::Type const *type = qt.getTypePtrOrNull();
      if (!type) {
        continue;
      }
      CXXRecordDecl *decl = type->getAsCXXRecordDecl();
      if (!decl || !ReadAnnotation(decl, nullptr)) {
        has_unannotated_base_class = true;

        if (verbose()) {
          outs() << "Has unannotated base class: " << decl->getName().str() << "\n";
        }
        continue;
      }
      if (super)
        continue;

      ASTRecordLayout const &layout =
          context_->getASTRecordLayout(D);
      base_class_offset = layout.getBaseClassOffset(decl).getQuantity();
      if (verbose()) {
        outs() << "base class offset: " << base_class_offset << "\n";
      }

      std::string record_name;
      llvm::raw_string_ostream os(record_name);
      os << *decl;
      os.flush();

      // nested classes are also available in bases, so check
      // if we didn't traverse too far
      if (parent) {
        if (record_name.compare(parent->name()) == 0)
          break;
      }

      Namespace *base_ns = GetOrCreateNamespaceForRecord(decl);
      if (!base_ns) {
        continue;
      }
      super = base_ns->FindClass(record_name.c_str());
      if (super)
        continue;
    }
    if (!has_unannotated_base_class)
      base_class_offset = 0;
  }

  SourceLocation source_loc = D->getSourceRange().getBegin();
  std::string header_file =
      PathRelativeToBaseDir(source_loc, src_manager(), basedir());

  PackageFile *pkg_file =
      package()->GetOrCreatePackageFile(header_file.c_str());


  Class *klass = new Class(name.c_str(), pkg_file, anno, super);
  klass->set_order(class_count());
  klass->set_base_class_offset(base_class_offset);
  set_class_count(class_count() + 1);
  if (!parent) {
    ns->AddClass(klass);
  }
  class_queue_.push_front(klass);

  bool ret = Base::TraverseDecl(D);

  class_queue_.pop_front();
  return ret;
}

bool ASTScanner::_TraverseFieldDecl(FieldDecl *D) {
  Annotation anno;
  if (!ReadAnnotation(D, &anno)) {
    return true;
  }

  if (D->isBitField() || D->isUnnamedBitfield()) {
    errs() << "bit-field types are not supported!";
    return true;
  }

  if (D->hasCapturedVLAType()) {
    errs() << "variable length array types are not supported!";
    return true;
  }

  if (D->isAnonymousStructOrUnion()) {
    errs() << "anonymous struct or union types are not supported!";
    return true;
  }

  LogDecl(D);

  std::string type_name;
  TypeQualifier tq;
	TypeRef tr;
  if (D->getTypeSourceInfo() &&
      !D->getTypeSourceInfo()->getTypeLoc().isNull()) {
    QualType qt = D->getType();
    clang::Type const *t = qt.getTypePtrOrNull();
    if (t) {
      tq.set_is_pointer(t->isPointerType());
      tq.set_is_ref(t->isReferenceType());
      tq.set_is_array(t->isArrayType());
    }
    tq.set_is_pod(qt.isPODType(*context_));
    tq.set_is_const(qt.isConstQualified());
    tq.set_is_volatile(qt.isVolatileQualified());
    tq.set_is_restrict(qt.isRestrictQualified());
    tq.set_is_mutable(D->isMutable());

    // construct proper field type name
    if (TypedefType::classof(t)) {
      TypedefNameDecl *TD = t->getAs<TypedefType>()->getDecl();
      if (TD->isCXXClassMember() && TD->getAccess() != AS_public) {
        // this is private typedef inside class, get underlying type
        // TODO maybe we should traverse underlying types until we find a
        // public one
        type_name = TD->getUnderlyingType().getAsString();
      } else {
        if (TD->isCXXClassMember()) {
          // this is public typedef inside class, get fully qualified name
          llvm::raw_string_ostream os(type_name);
          TD->printQualifiedName(os);
          os.flush();
        }
      }
    } else if (RecordType::classof(t)) {
      RecordDecl *RD = t->getAs<RecordType>()->getDecl();
      if (RD->isCXXClassMember()) {
        llvm::raw_string_ostream os(type_name);
        RD->printQualifiedName(os);
        os.flush();
      }
    } else if (EnumType::classof(t)) {
      EnumDecl *ED = t->getAs<EnumType>()->getDecl();
      if (ED->isCXXClassMember()) {
        llvm::raw_string_ostream os(type_name);
        ED->printQualifiedName(os);
        os.flush();
      } else {
				Namespace *ns = GetOrCreateNamespaceForRecord(ED);
				Enum *enm = ns->FindEnum(ED->getName().data());
				if (enm == nullptr) {
					errs() << "Could not find enum ";
					ED->printQualifiedName(errs());
					errs() << "\n";
					errs().flush();
				} else {
					tr.set_enum_type(enm);
				}
			}
    }

    if (type_name.empty()) {
      PrintingPolicy policy(context_->getLangOpts());
      policy.SuppressTagKeyword = true;
      policy.SuppressScope = true;
      type_name = D->getType().getAsString(policy);
    }
		if (tr.kind() == TypeRef::kInvalid_Kind)
			tr.set_type_name(type_name.c_str());
  } else {
    errs() << "Error missing TypeSourceInfo "
           << D->getType().getLocalUnqualifiedType().getAsString() << "\n";
    return true;
  }

  std::string field_name = D->getDeclName().getAsString();

  Class *klass = CurrentClass();
  if (!klass) {
    errs() << "Error no enclosing class for " << field_name.c_str() << "\n";
    return true;
  }

  ASTRecordLayout const &layout = context_->getASTRecordLayout(D->getParent());
  uint32 offset = (uint32)layout.getFieldOffset(D->getFieldIndex());

  klass->AddField(
      new Field(field_name.c_str(), tr, offset, tq, anno));
  return true;
}

bool ASTScanner::_TraverseEnumDecl(EnumDecl *D) {
  Annotation anno;
  if (!ReadAnnotation(D, &anno)) {
    return true;
  }

  std::string name = D->getQualifiedNameAsString();
  name = D->getName().str();

  QualType intQT = D->getIntegerType().getLocalUnqualifiedType();
  std::string type = intQT.getAsString();

  Class *parent = CurrentClass();
  Namespace *ns = nullptr;
  if (parent == nullptr) {
    ns = GetOrCreateNamespaceForRecord(D);
    if (!ns || ns->FindEnum(name.c_str()) != nullptr)
      return true;
  } else if (parent->FindEnum(name.c_str()) != nullptr){
      return true;
  }

  LogDecl(D);

  SourceLocation source_loc = D->getSourceRange().getBegin();
  std::string header_file = PathRelativeToBaseDir(source_loc, src_manager(), basedir());

  PackageFile *pkg_file =
      package()->GetOrCreatePackageFile(header_file.c_str());
  Enum *e = new Enum(name.c_str(), type.c_str(), pkg_file, anno, ns, parent);

  // collect enum items
  for (EnumDecl::enumerator_iterator it = D->enumerator_begin();
       it != D->enumerator_end(); ++it) {
    EnumConstantDecl *e_item = *it;
    EnumItem item;
    item.set_id(e_item->getNameAsString().c_str());
    llvm::APSInt const &value = e_item->getInitVal();
    item.set_value((long) value.getSExtValue());
    item.set_name(anno.GetEntry(item.id())); // XXX check
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

  LogDecl(D);

  std::string name = D->getDeclName().getAsString();
  PrintingPolicy policy(context_->getLangOpts());
  policy.SuppressTagKeyword = true;
  policy.SuppressScope = true;
  std::string ret_type =
      D->getReturnType().getLocalUnqualifiedType().getAsString();

  Class *klass = class_queue_.front();
  Method *method = new Method(name.c_str(), anno);
  // TODO check that method does not exists, handle overloads
  klass->AddMethod(method);
  method->AddArgument(
      new Argument("return", Argument::kReturn_Kind, ret_type.c_str(), Annotation()));

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

    Argument *arg =
        new Argument(param_name.data(), kind, param_type.c_str(), param_anno);
    method->AddArgument(arg);
  }

  return true;
}

void ASTScanner::AddDecl(NamedDecl *D) {
  std::string name = D->getQualifiedNameAsString();
  Annotation anno;
  if (ReadAnnotation(D, &anno)) {
    if (verbose()) {
      D->getLocStart().print(outs(), src_manager());
      outs() << " : Found unknown annotated declaration\n";
      D->print(outs(), PrintingPolicy(context_->getLangOpts()));
      outs() << "\n";
      outs().flush();
    }
  }
}

void ASTScanner::LogDecl(NamedDecl *D) const {
  if (verbose()) {
    D->getLocation().print(outs(), src_manager());
    outs() << " : ";
    PrintingPolicy policy(context_->getLangOpts());
    policy.SuppressUnwrittenScope = true;
    if (verbose() < 3) {
      policy.TerseOutput = true;
      policy.PolishForDeclaration = true;
    }
    if (verbose() < 2) {
      policy.SuppressScope = true;
    }
    D->print(outs(), policy);
    outs() << "\n";
    outs().flush();
  }
}

SourceManager const &ASTScanner::src_manager() const {
  return context_->getSourceManager();
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
