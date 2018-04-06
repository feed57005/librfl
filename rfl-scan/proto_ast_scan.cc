// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "proto_ast_scan.h"

#include "rfl/reflected.h"
#include "rfl-scan/annotation_parser.h"

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

namespace rfl {
namespace scan {

namespace {

struct AnnoInserter {
  AnnoInserter(proto::Annotation *anno) : anno_(anno) {}

  void operator()(std::string const &key, std::string const &value) const {
    proto::Annotation_Entry *entry = anno_->add_entries();
    entry->set_key(key);
    entry->set_value(value);
  }

  proto::Annotation *anno_;
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

Scanner::Scanner(ScannerContext *scan_ctx, raw_ostream *out)
    : scanner_context_(scan_ctx),
      context_(nullptr),
      out_(out ? *out : outs()),
      current_file_(nullptr) {}

void Scanner::HandleTranslationUnit(ASTContext &Context) {
  TranslationUnitDecl *D = Context.getTranslationUnitDecl();
  context_ = &Context;

  FileID main_id = src_manager().getMainFileID();
  current_file_location_ = src_manager().getLocForStartOfFile(main_id);
  std::string file_location =
      PathRelativeToBaseDir(current_file_location_, src_manager(), basedir());
  current_file_ = package().add_package_files();
  current_file_->set_name(file_location);
  if (verbose()) {
    outs() << "Translation unit: ";
    current_file_location_.print(outs(), src_manager());
    outs() << "\n";
  }
  Base::TraverseDecl(D);
}

SourceManager const &Scanner::src_manager() const {
  return context_->getSourceManager();
}

void Scanner::LogDecl(NamedDecl *D) const {
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

bool Scanner::HasAnnotation(NamedDecl *D) const {
  specific_attr_iterator<AnnotateAttr> i =
      D->specific_attr_begin<AnnotateAttr>();
  if (i == D->specific_attr_end<AnnotateAttr>()) {
    return false;
  }
  return true;
}

bool Scanner::ReadAnnotation(NamedDecl *D, proto::Annotation *anno) {
  // Reflection attributes are stored as clang annotation attributes
  if (!HasAnnotation(D))
    return false;

  // Get the annotation text
  AnnotateAttr *attribute = *D->specific_attr_begin<AnnotateAttr>();
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

bool Scanner::TraverseNamespaceDecl(NamespaceDecl *D) {
  // skip declarations not in this translation unit
  if (!IsCurrentFileLocation(D->getLocation()))
    return true;

  if (!Base::TraverseNamespaceDecl(D))
    return false;

  namespace_queue_.pop_front();

  return true;
}

bool Scanner::VisitNamespaceDecl(NamespaceDecl *D) {
  proto::Namespace *parent_ns = CurrentNamespace();
  proto::Namespace *ns = nullptr;
  if (!parent_ns) {
    ns = current_file_->add_namespaces();
  } else {
    ns = parent_ns->add_namespaces();
  }
  ns->set_name(D->getDeclName().getAsString());
  namespace_queue_.push_front(ns);
  return true;
}

bool Scanner::IsCurrentFileLocation(SourceLocation loc) const {
  FileID current_file_id = src_manager().getFileID(current_file_location_);
  // if location is not a macro compare FileIDs
  if (!loc.isMacroID() && src_manager().getFileID(loc) == current_file_id)
    return true;

  // otherwise check the expansion location
  SourceLocation expansion = src_manager().getExpansionLoc(loc);
  if (src_manager().getFileID(expansion) == current_file_id)
    return true;
  return false;
}

bool Scanner::TraverseCXXRecordDecl(CXXRecordDecl *D) {
  if (!IsCurrentFileLocation(D->getLocation()) ||
      D->isThisDeclarationADefinition() == VarDecl::DeclarationOnly) {
    return true;
  }

  if (!HasAnnotation(D)) {
    return true;
  }

  if (!Base::TraverseCXXRecordDecl(D))
    return false;

  class_queue_.pop_front();
  return true;
}

bool Scanner::VisitCXXRecordDecl(CXXRecordDecl *D) {
  // skip declarations not in this translation unit
  if (!IsCurrentFileLocation(D->getLocation())) {
    return true;
  }
  proto::Annotation anno;
  if (!ReadAnnotation(D, &anno)) {
    return true;
  }

  LogDecl(D);
  outs().flush();

  proto::Class *parent = CurrentClass();
  proto::Namespace *ns = nullptr;
  if (!parent) {
    ns = CurrentNamespace();
  }
  proto::Class *klass;
  if (parent) {
    klass = parent->add_classes();
  } else if (ns) {
    klass = ns->add_classes();
  } else {
    klass = current_file_->add_classes();
  }

  if (D->getNumBases()) {
    for (CXXBaseSpecifier const &base : D->bases()) {
      QualType qt = base.getType();

      clang::Type const *type = qt.getTypePtrOrNull();
      if (!type) {
        continue;
      }
      CXXRecordDecl *decl = type->getAsCXXRecordDecl();
      if (!decl || !ReadAnnotation(decl, nullptr)) {
        continue;
      }

      std::string record_name;
      llvm::raw_string_ostream os(record_name);
      os << *decl;
      os.flush();

      proto::TypeRef base_class;
      SourceLocation base_class_location = decl->getSourceRange().getBegin();
      std::string header_file =
          PathRelativeToBaseDir(base_class_location, src_manager(), basedir());
      base_class.set_type_name(decl->getQualifiedNameAsString());
      base_class.set_kind(proto::TypeRef_Kind_CLASS);
      base_class.set_source_file(header_file);
      klass->mutable_base_class()->CopyFrom(base_class);
    }
  }
  klass->set_name(D->getDeclName().getAsString());
  klass->set_order(class_count());
  klass->mutable_annotation()->CopyFrom(anno);
  set_class_count(class_count() + 1);
  class_queue_.push_front(klass);
  package().add_provided_classes(D->getQualifiedNameAsString());
  return true;
}

bool Scanner::VisitFieldDecl(FieldDecl *D) {
  proto::Annotation anno;
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

  std::string field_name = D->getDeclName().getAsString();

  proto::Class *klass = CurrentClass();
  if (!klass) {
    errs() << "Error no enclosing class for " << field_name.c_str() << "\n";
    return true;
  }

  ASTRecordLayout const &layout = context_->getASTRecordLayout(D->getParent());
  uint32 offset = (uint32)layout.getFieldOffset(D->getFieldIndex());

  proto::Field *field = klass->add_fields();
  field->set_name(field_name);
  ReadType(D->getType(), field->mutable_type_ref(), field->mutable_type_qualifier());
  field->set_offset(offset);
  field->mutable_annotation()->CopyFrom(anno);
  return true;
}

bool Scanner::VisitFunctionDecl(FunctionDecl *D) {
  if (D->getKind() == Decl::CXXMethod)
    return true;

  proto::Annotation anno;
  if (!ReadAnnotation(D, &anno))
    return true;

  LogDecl(D);

  std::string func_name = D->getDeclName().getAsString();
  proto::Namespace *ns = CurrentNamespace();
  if (!ns) {
    errs() << "Error no enclosing namespace for function " << func_name << "\n";
    return true;
  }

  proto::Function *func = ns->add_functions();
  func->mutable_annotation()->CopyFrom(anno);
  func->set_name(func_name);

  proto::Argument *ret_val = func->mutable_return_value();
  ret_val->set_name("return");
  ReadType(D->getReturnType(), ret_val->mutable_type_ref(),
           ret_val->mutable_type_qualifier());

  for (FunctionDecl::param_iterator it = D->param_begin();
       it != D->param_end(); ++it) {
    ParmVarDecl *param = *it;
    proto::Argument *arg = func->add_arguments();
    ReadAnnotation(param, arg->mutable_annotation());

    llvm::StringRef param_name = param->getName().str();
    arg->set_name(param_name.data());
    ReadType(param->getType(), arg->mutable_type_ref(),
             arg->mutable_type_qualifier());
  }
  return true;
}

bool Scanner::VisitCXXMethodDecl(CXXMethodDecl *D) {
  // Ignore overloaded operators for now
  if (D->isOverloadedOperator())
    return true;

  proto::Annotation anno;
  if (!ReadAnnotation(D, &anno))
    return true;

  LogDecl(D);

  std::string method_name = D->getDeclName().getAsString();
  proto::Class *klass = CurrentClass();
  if (!klass) {
    errs() << "Error no enclosing class for method " << method_name.c_str()
           << "\n";
    return true;
  }

  proto::Method *method = klass->add_methods();
  method->mutable_annotation()->CopyFrom(anno);
  method->set_name(method_name);
  method->set_static_method(D->isStatic());

  proto::Argument *ret_val = method->mutable_return_value();
  ret_val->set_name("return");
  ReadType(D->getReturnType(), ret_val->mutable_type_ref(),
           ret_val->mutable_type_qualifier());

  for (CXXMethodDecl::param_iterator it = D->param_begin();
       it != D->param_end(); ++it) {
    ParmVarDecl *param = *it;
    proto::Argument *arg = method->add_arguments();
    ReadAnnotation(param, arg->mutable_annotation());

    llvm::StringRef param_name = param->getName().str();
    arg->set_name(param_name.data());
    ReadType(param->getType(), arg->mutable_type_ref(),
             arg->mutable_type_qualifier());
  }

  return true;
}

bool Scanner::VisitTypedefDecl(TypedefDecl *D) {
  proto::Annotation anno;
  if (!ReadAnnotation(D, &anno))
    return true;

  LogDecl(D);

  std::string td_name = D->getDeclName().getAsString();

  proto::Typedef *td = nullptr;
  proto::Class *klass = CurrentClass();
  if (klass) {
    td = klass->add_typedefs();
  } else {
    proto::Namespace *ns = CurrentNamespace();
    if (ns) {
      td = ns->add_typedefs();
    } else {
      errs() << "Error no enclosing namespace for " << td_name << "\n";
      return true;
    }
  }

  td->set_name(td_name);
  td->mutable_annotation()->CopyFrom(anno);

  if (!ReadType(D->getUnderlyingType(), td->mutable_type_ref(),
                td->mutable_type_qualifier())) {
    errs() << "Failed to read underlying type for " << td_name << "\n";
    // FIXME delete invalid td
    return true;
  }

  return true;
}

bool Scanner::ReadType(QualType qt,
                       proto::TypeRef *tr,
                       proto::TypeQualifier *tq) {
  clang::Type const *t = qt.getTypePtrOrNull();
  if (t) {
    tq->set_is_pointer(t->isPointerType());
    tq->set_is_ref(t->isReferenceType());
    tq->set_is_array(t->isArrayType());
  }
  tq->set_is_pod(qt.isPODType(*context_));
  tq->set_is_const(qt.isConstQualified());
  tq->set_is_volatile(qt.isVolatileQualified());
  tq->set_is_restrict(qt.isRestrictQualified());

  std::string type_name;
  llvm::raw_string_ostream os(type_name);

  if (verbose() > 1) {
    llvm::outs() << t->getTypeClassName() << "\n";
    llvm::outs().flush();
  }

  if (BuiltinType::classof(t)) {
    PrintingPolicy policy(context_->getLangOpts());
    policy.SuppressTagKeyword = true;
    policy.SuppressScope = false;
    type_name = qt.getAsString(policy);
    tr->set_kind(proto::TypeRef_Kind_SYSTEM);
  } else if (TypedefType::classof(t)) {
    // Get underlying type for typedefs
    TypedefNameDecl *TD = t->getAs<TypedefType>()->getDecl();

    // TODO we should traverse underlying types until we find a public one
    tr->set_underlying_type(TD->getUnderlyingType().getAsString());

    if (TD->isCXXClassMember() && TD->getAccess() != AS_public) {
      // this is private typedef inside class, use the underlying type
      type_name = TD->getUnderlyingType().getAsString();  // TODO
    } else {
      // this is public typedef, get fully qualified name
      TD->printQualifiedName(os);
      os.flush();
    }
  } else if (RecordType::classof(t)) {
    RecordDecl *RD = t->getAs<RecordType>()->getDecl();
    RD->printQualifiedName(os);
    os.flush();
    tr->set_kind(proto::TypeRef_Kind_CLASS);

    SourceLocation location = RD->getSourceRange().getBegin();
    tr->set_source_file(
        PathRelativeToBaseDir(location, src_manager(), basedir()));
  } else if (ElaboratedType::classof(t)) {
    return ReadType(t->getAs<ElaboratedType>()->getNamedType(), tr,tq);
  } else if (EnumType::classof(t)) {
    EnumDecl *ED = t->getAs<EnumType>()->getDecl();
    llvm::raw_string_ostream os(type_name);
    ED->printQualifiedName(os);
    os.flush();
    tr->set_kind(proto::TypeRef_Kind_ENUM);

    SourceLocation location = ED->getSourceRange().getBegin();
    tr->set_source_file(
        PathRelativeToBaseDir(location, src_manager(), basedir()));
  } else if (clang::PointerType::classof(t)) {
    PrintingPolicy policy(context_->getLangOpts());
    policy.SuppressTagKeyword = true;
    policy.SuppressScope = false;
    type_name = t->getAs<clang::PointerType>()->getPointeeType().getAsString(policy);
    tr->set_kind(proto::TypeRef_Kind_SYSTEM);
  } else {
    PrintingPolicy policy(context_->getLangOpts());
    policy.SuppressTagKeyword = true;
    policy.SuppressScope = false;
    type_name = qt.getAsString(policy);
    tr->set_kind(proto::TypeRef_Kind_INVALID);
  }

  tr->set_type_name(type_name.c_str());

  return true;
}

bool Scanner::VisitEnumDecl(EnumDecl *D) {
  // skip declarations not in this translation unit
  if (!IsCurrentFileLocation(D->getLocation()))
    return true;

  proto::Annotation anno;
  if (!ReadAnnotation(D, &anno)) {
    return true;
  }

  LogDecl(D);

  proto::Class *parent = CurrentClass();
  proto::Namespace *ns = nullptr;
  if (!parent) {
    ns = CurrentNamespace();
  }
  proto::Enum *enm;
  if (parent) {
    enm = parent->add_enums();
  } else if (ns) {
    enm = ns->add_enums();
  } else {
    enm = current_file_->add_enums();
  }

  enm->set_name(D->getName().str());
  enm->set_type(D->getIntegerType().getLocalUnqualifiedType().getAsString());
  enm->mutable_annotation()->CopyFrom(anno);

  for (EnumDecl::enumerator_iterator it = D->enumerator_begin();
       it != D->enumerator_end(); ++it) {
    EnumConstantDecl *e_item = *it;
    proto::Enum_Item *item = enm->add_items();

    llvm::APSInt const &value = e_item->getInitVal();
    item->set_value(value.getSExtValue());
    item->set_id(e_item->getNameAsString());
  }

  return true;
}

} // namespace scan
} // namespace rfl
