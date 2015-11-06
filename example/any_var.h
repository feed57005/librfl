// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __EXAMPLE_ANY_VAR_H__
#define __EXAMPLE_ANY_VAR_H__

#include "example/example_export.h"
#include "example/type_info.h"
#include <new>

namespace example {

struct AnyVarModel;

/**
 * @internal AnyVarVTable
 * Low-level virtual table structure for work with regular types.
 * This structure is filled when templated AnyVarModel is instantiated.
 */
struct EXAMPLE_EXPORT AnyVarVTable {
  // operator= (T const &);
  void (*Assign)(AnyVarModel &dst, AnyVarModel const &src);

  // ~T()
  void (*Destroy)(AnyVarModel const &model);

  // new T (T const &);
  AnyVarModel *(*Clone)(AnyVarModel const &src, void *storage);

  // ~T()
  // new T (T const &src)
  AnyVarModel *(*MoveClone)(AnyVarModel &src,
                            void *storage);  // AnyVar& operator= (AnyVar)

  // operator== (T const &, T const &)
  bool (*Equals)(AnyVarModel const &a, AnyVarModel const &b);

  // TypeInfoOf<T>()
  TypeInfo (*GetType)();
};

//- AnyVarModel
//---------------------------------------------------------------{{{
/**
 * @internal AnyVarModel
 * Base structure for templated model implementations + convinience wrapper for
 * AnyVarVTable calling.
 */
struct EXAMPLE_EXPORT AnyVarModel {
  AnyVarModel(AnyVarVTable const &table) : m_VTable(&table) {}

  inline void Assign(AnyVarModel const &model) {
    m_VTable->Assign(*this, model);
  }

  inline void Destroy() const { m_VTable->Destroy(*this); }

  inline AnyVarModel *Clone(void *x) const { return m_VTable->Clone(*this, x); }

  inline AnyVarModel *MoveClone(void *x) {
    return m_VTable->MoveClone(*this, x);
  }

  inline bool Equals(AnyVarModel const &x) const {
    return m_VTable->Equals(*this, x);
  }

  inline TypeInfo GetType() const { return m_VTable->GetType(); }

  AnyVarVTable const *m_VTable;
};
//---------------------------------------------------------------------------}}}

/*- AnyVar Local Model {{{
 * ----------------------------------------------------*/

/**
 * @internal AnyVarLocalModel
 * Stores data as value.
 */
template <typename T>
struct AnyVarLocalModel : AnyVarModel {
  AnyVarLocalModel() : AnyVarModel(vtable_), data_() {}

  explicit AnyVarLocalModel(T const &value)
      : AnyVarModel(vtable_), data_(value) {}

  T &Get() { return data_; }

  const T &Get() const { return data_; }

  static void Assign(AnyVarModel &dst, AnyVarModel const &src) {
    static_cast<AnyVarLocalModel &>(dst).data_ =
        static_cast<AnyVarLocalModel const &>(src).data_;
  }

  static void Destroy(AnyVarModel const &model) {
    static_cast<AnyVarLocalModel const &>(model).~AnyVarLocalModel();
  }

  static AnyVarModel *Clone(AnyVarModel const &x, void *storage) {
    return new (storage)
        AnyVarLocalModel<T>(static_cast<AnyVarLocalModel const &>(x).data_);
  }

  static AnyVarModel *MoveClone(AnyVarModel &x, void *storage) {
    // static_cast <T *>(storage)->~T();
    return new (storage)
        AnyVarLocalModel(static_cast<AnyVarLocalModel &>(x).data_);
  }

  static bool Equals(AnyVarModel const &a, AnyVarModel const &b) {
    return static_cast<const AnyVarLocalModel &>(a).data_ ==
           static_cast<AnyVarLocalModel const &>(b).data_;
  }

  static TypeInfo GetType() { return TypeInfoOf<T>(); }

  static AnyVarVTable const vtable_;
  T data_;
};

template <typename T>
AnyVarVTable const AnyVarLocalModel<T>::vtable_ = {&AnyVarLocalModel::Assign,
                                                   &AnyVarLocalModel::Destroy,
                                                   &AnyVarLocalModel::Clone,
                                                   &AnyVarLocalModel::MoveClone,
                                                   &AnyVarLocalModel::Equals,
                                                   &AnyVarLocalModel::GetType};
/* }}} */

/*- AnyVar Remote Model {{{
 * ---------------------------------------------------*/

/**
 * @internal AnyVarRemoteModel
 * Stores data as pointer to value.
 */
template <typename T>
struct AnyVarRemoteModel : AnyVarModel {
  AnyVarRemoteModel() : AnyVarModel(vtable_), remote_(NULL) {}

  explicit AnyVarRemoteModel(T data) : AnyVarModel(vtable_) {
    remote_ = new Remote();
    new (static_cast<void *>(&remote_->Data)) T(data);
  }

  ~AnyVarRemoteModel() {
    if (remote_) {
      delete remote_;
      remote_ = 0;
    }
  }

  T &Get() { return remote_->Data; }

  T const &Get() const { return remote_->Data; }

  static void Assign(AnyVarModel &dst, AnyVarModel const &src) {
    static_cast<AnyVarRemoteModel &>(dst).remote_ =
        static_cast<AnyVarRemoteModel const &>(src).remote_;
  }

  static void Destroy(AnyVarModel const &model) {
    static_cast<AnyVarRemoteModel const &>(model).~AnyVarRemoteModel();
  }

  // FIXME create & use copy constructor
  static AnyVarModel *Clone(AnyVarModel const &x, void *storage) {
    return new (storage) AnyVarRemoteModel(
        static_cast<AnyVarRemoteModel const &>(x).remote_->Data);
  }

  static AnyVarModel *MoveClone(AnyVarModel &x, void *storage) {
    // static_cast <T *>(storage)->~T();
    return new (storage)
        AnyVarRemoteModel(static_cast<AnyVarRemoteModel &>(x).remote_->Data);
  }

  static bool Equals(AnyVarModel const &a, AnyVarModel const &b) {
    return static_cast<AnyVarRemoteModel const &>(a).remote_->Data ==
           static_cast<AnyVarRemoteModel const &>(b).remote_->Data;
  }

  static TypeInfo GetType() { return TypeInfoOf<T>(); }

  static AnyVarVTable const vtable_;

  // wrapper structure so that we can take reference
  // of modelled T
  struct Remote {
    T Data;
    Remote() {}
  };

  Remote *remote_;
};

template <typename T>
AnyVarVTable const AnyVarRemoteModel<T>::vtable_ = {
    &AnyVarRemoteModel::Assign,
    &AnyVarRemoteModel::Destroy,
    &AnyVarRemoteModel::Clone,
    &AnyVarRemoteModel::MoveClone,
    &AnyVarRemoteModel::Equals,
    &AnyVarRemoteModel::GetType};
/* }}} */

/*---------------------------------------------------------------------------*/

/**
 * AnyVar
 * generic polymorphic container type that can hold values of any other
 * types. Contained types must meet the following conditions :
 *  - has copy and default constructors
 *  - is assignable (operator =)
 *  - is equality comparable (operator ==, !=)
 *
 * Model is chosen at compile time depending on the sizeof type being
 * stored.
 */
class AnyVar {
  /** sizeof available memory for Models */
  typedef intptr_t AnyVarStorage[2];

  /* compile time Model type selection */
  template <typename T>
  struct Traits {
    static const int IsLocal =
        sizeof(AnyVarLocalModel<T>) <= sizeof(AnyVarStorage);

    template <bool B, typename T1, typename T2>
    struct TypeSelect {
      typedef T1 Type;
    };

    template <typename T1, typename T2>
    struct TypeSelect<false, T1, T2> {
      typedef T2 Type;
    };

    typedef typename TypeSelect<IsLocal,
                                AnyVarLocalModel<T>,
                                AnyVarRemoteModel<T> >::Type ModelType;
  };

public:
  AnyVar() { new ((void *)&storage_) Traits<EmptyType>::ModelType(); }

  template <typename T>
  AnyVar(T const &value) {
    new ((void *)&storage_) typename Traits<T>::ModelType(value);
  }

  AnyVar(AnyVar const &x) { x.GetModel().Clone((void *)&storage_); }

  ~AnyVar() { GetModel().Destroy(); }

  inline AnyVar &operator=(AnyVar x) {
    GetModel().Destroy();
    x.GetModel().MoveClone((void *)&storage_);
    return *this;
  }

  inline bool operator==(AnyVar const &x) const {
    return (x.GetType() == GetType() && GetModel().Equals(x.GetModel()));
  }

  /** @return TypeInfo held by this instance */
  TypeInfo GetType() const { return GetModel().GetType(); }

  /**
   * Get value held by this instance.
   * @param val where to store value
   * @return false when types doesn't match
   */
  template <typename T>
  bool GetValue(T &val) const {
    // perform type check
    if (TypeInfoOf<T>() != GetType())
      return false;

    val = static_cast<T const &>(
        reinterpret_cast<typename Traits<T>::ModelType const &>(GetModel())
            .Get());

    return true;
  }

  /**
   * Type unsafe cast.
   */
  template <typename T>
  T const &Cast() const {
    return static_cast<T const &>(
        reinterpret_cast<typename Traits<T>::ModelType const &>(GetModel())
            .Get());
  }

  /**
   * Type unsafe Cast.
   */
  template <typename T>
  T &Cast() {
    return static_cast<T &>(
        reinterpret_cast<typename Traits<T>::ModelType &>(GetModel()).Get());
  }

  /** @return true when this instance helds an empty type */
  inline bool IsEmpty() const { return GetType() == TypeInfoOf<EmptyType>(); }

  static AnyVar const &empty() {
    static AnyVar empty_var;
    return empty_var;
  }

private:
  inline AnyVarModel &GetModel() {
    return *static_cast<AnyVarModel *>((void *)&storage_);
  }

  inline AnyVarModel const &GetModel() const {
    return *static_cast<AnyVarModel const *>((void *)&storage_);
  }

  AnyVarStorage storage_;
};

}  // namespace example

#endif /* __EXAMPLE_ANY_VAR_H__ */
