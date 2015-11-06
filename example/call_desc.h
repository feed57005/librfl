// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __EXAMPLE_CALL_DESC_H__
#define __EXAMPLE_CALL_DESC_H__

#include "example/type_info.h"
#include "rfl/types.h"
#include <new>

namespace example {

using rfl::uint8;

struct VStack {
  static size_t const kInvalidOffset = size_t(-1);
  uint8 *stack_;

  VStack() : stack_(NULL) {}

  ~VStack() {
    if (stack_)
      delete[] stack_;
  }

  void Create(size_t stack_size) {
    if (stack_)
      delete[] stack_;
    stack_ = new uint8[stack_size];
  }

  void *At(size_t offset) {
    return reinterpret_cast<void *>(
        (reinterpret_cast<size_t>(stack_) + offset));
  }
};

struct CallDesc {
  typedef void (*ExecFunc)(CallDesc const *spec, VStack *stack, void *node,
                           size_t *stack_offsets);

  char const *signature_;
  size_t const *sizes_;
  TypeInfo const *types_;
  size_t const num_;
  ExecFunc const execute_;

  CallDesc(char const *signature, size_t const *sizes, TypeInfo const *types,
      size_t const num,
           ExecFunc const exec_func)
      : signature_(signature), sizes_(sizes), types_(types), num_(num),
          execute_(exec_func) {}

};
////////////////////////////////////////////////////////////////////////////////
namespace detail {
template <typename T> struct CallArgTrait {
  template <class U> struct reference_traits {
    typedef U *reftype;
  };

  template <class U> struct reference_traits<U &> {
    typedef U *reftype;
  };

  template <class U> struct reference_traits<U *> {
    typedef U **reftype;
  };

  typedef typename reference_traits<T>::reftype CastType;

  template <class U> struct plain_traits {
    typedef U plain_type;
  };
  template <class U> struct plain_traits<U &> {
    typedef U plain_type;
  };
  template <class U> struct plain_traits<U *> {
    typedef U plain_type;
  };
  typedef typename plain_traits<T>::plain_type PlainType;
};
} // namespace detail

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename Signature> struct GenericCallDesc;
template <typename T>
struct GenericCallDesc<T, void(T::*)(void)> : public CallDesc {
  typedef GenericCallDesc<T, void(T::*)(void)> ThisSpec;
  typedef void (T::*MemberFn)(void);

  GenericCallDesc(MemberFn member_fn, char const *signature)
      : CallDesc(signature, t_sizes_, t_types_, 1, &Exec),
          member_fn_(member_fn) {}

  static void Exec(CallDesc const *spec, VStack *stack, void *node,
                   size_t *stack_offsets) {
    ThisSpec *this_spec = (ThisSpec *)spec;
    using namespace detail;
    T *the_node = (T *) node;
    (the_node->*(this_spec->member_fn_))();
  }

  MemberFn const member_fn_;
  static size_t const t_sizes_[1];
  static TypeInfo const t_types_[1];
};

template <class T>
size_t const GenericCallDesc<T, void(T::*)(void)>::t_sizes_[1] = { 0 };

template <class T>
TypeInfo const GenericCallDesc<T, void(T::*)()>::t_types_[1] = {
  TypeInfoOf<EmptyType>()
};
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename A1>
struct GenericCallDesc<T, void(T::*)(A1)> : public CallDesc {
  typedef GenericCallDesc<T, void(T::*)(A1)> ThisSpec;
  typedef void(T::*MemberFn)(A1);

  GenericCallDesc(MemberFn member_fn, char const *signature)
      : CallDesc(signature, t_sizes_, t_types_, 1+1, &Exec),
          member_fn_(member_fn) {}

  static void Exec(CallDesc const *spec, VStack *stack, void *node,
                   size_t *stack_offsets) {
    ThisSpec *this_spec = (ThisSpec *)spec;
    using namespace detail;
    T *the_node = (T *) node;
    void *mem_a1 = stack->At(stack_offsets[1]);

    A1 a1 = *((typename CallArgTrait<A1>::CastType) mem_a1);
      (the_node->*(this_spec->member_fn_))(a1);
  }

  MemberFn const member_fn_;
  static size_t const t_sizes_[1+1];
  static TypeInfo const t_types_[1+1];
};

template <class T, typename A1>
size_t const GenericCallDesc<T, void(T::*)(A1)>::t_sizes_[1+1] = { 0,
    sizeof(A1) };

template <class T, typename A1>
TypeInfo const GenericCallDesc<T, void(T::*)(A1)>::t_types_[1+1] = {
  TypeInfoOf<EmptyType>(),
      TypeInfoOf<typename detail::CallArgTrait<A1>::PlainType>()};
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename A1, typename A2>
struct GenericCallDesc<T, void(T::*)(A1, A2)> : public CallDesc {
  typedef GenericCallDesc<T, void(T::*)(A1, A2)> ThisSpec;
  typedef void(T::*MemberFn)(A1, A2);

  GenericCallDesc(MemberFn member_fn, char const *signature)
      : CallDesc(signature, t_sizes_, t_types_, 2+1, &Exec),
          member_fn_(member_fn) {}

  static void Exec(CallDesc const *spec, VStack *stack, void *node,
                   size_t *stack_offsets) {
    ThisSpec *this_spec = (ThisSpec *)spec;
    using namespace detail;
    T *the_node = (T *) node;
    void *mem_a1 = stack->At(stack_offsets[1]);

    A1 a1 = *((typename CallArgTrait<A1>::CastType) mem_a1);

    void *mem_a2 = stack->At(stack_offsets[2]);

    A2 a2 = *((typename CallArgTrait<A2>::CastType) mem_a2);
      (the_node->*(this_spec->member_fn_))(a1, a2);
  }

  MemberFn const member_fn_;
  static size_t const t_sizes_[2+1];
  static TypeInfo const t_types_[2+1];
};

template <class T, typename A1, typename A2>
size_t const GenericCallDesc<T, void(T::*)(A1, A2)>::t_sizes_[2+1] = { 0,
    sizeof(A1), sizeof(A2) };

template <class T, typename A1, typename A2>
TypeInfo const GenericCallDesc<T, void(T::*)(A1, A2)>::t_types_[2+1] = {
  TypeInfoOf<EmptyType>(),
      TypeInfoOf<typename detail::CallArgTrait<A1>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A2>::PlainType>()};
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename A1, typename A2, typename A3>
struct GenericCallDesc<T, void(T::*)(A1, A2, A3)> : public CallDesc {
  typedef GenericCallDesc<T, void(T::*)(A1, A2, A3)> ThisSpec;
  typedef void(T::*MemberFn)(A1, A2, A3);

  GenericCallDesc(MemberFn member_fn, char const *signature)
      : CallDesc(signature, t_sizes_, t_types_, 3+1, &Exec),
          member_fn_(member_fn) {}

  static void Exec(CallDesc const *spec, VStack *stack, void *node,
                   size_t *stack_offsets) {
    ThisSpec *this_spec = (ThisSpec *)spec;
    using namespace detail;
    T *the_node = (T *) node;
    void *mem_a1 = stack->At(stack_offsets[1]);

    A1 a1 = *((typename CallArgTrait<A1>::CastType) mem_a1);

    void *mem_a2 = stack->At(stack_offsets[2]);

    A2 a2 = *((typename CallArgTrait<A2>::CastType) mem_a2);

    void *mem_a3 = stack->At(stack_offsets[3]);

    A3 a3 = *((typename CallArgTrait<A3>::CastType) mem_a3);
      (the_node->*(this_spec->member_fn_))(a1, a2, a3);
  }

  MemberFn const member_fn_;
  static size_t const t_sizes_[3+1];
  static TypeInfo const t_types_[3+1];
};

template <class T, typename A1, typename A2, typename A3>
size_t const GenericCallDesc<T, void(T::*)(A1, A2, A3)>::t_sizes_[3+1] = { 0,
    sizeof(A1), sizeof(A2), sizeof(A3) };

template <class T, typename A1, typename A2, typename A3>
TypeInfo const GenericCallDesc<T, void(T::*)(A1, A2, A3)>::t_types_[3+1] = {
  TypeInfoOf<EmptyType>(),
      TypeInfoOf<typename detail::CallArgTrait<A1>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A2>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A3>::PlainType>()};
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename A1, typename A2, typename A3, typename A4>
struct GenericCallDesc<T, void(T::*)(A1, A2, A3, A4)> : public CallDesc {
  typedef GenericCallDesc<T, void(T::*)(A1, A2, A3, A4)> ThisSpec;
  typedef void(T::*MemberFn)(A1, A2, A3, A4);

  GenericCallDesc(MemberFn member_fn, char const *signature)
      : CallDesc(signature, t_sizes_, t_types_, 4+1, &Exec),
          member_fn_(member_fn) {}

  static void Exec(CallDesc const *spec, VStack *stack, void *node,
                   size_t *stack_offsets) {
    ThisSpec *this_spec = (ThisSpec *)spec;
    using namespace detail;
    T *the_node = (T *) node;
    void *mem_a1 = stack->At(stack_offsets[1]);

    A1 a1 = *((typename CallArgTrait<A1>::CastType) mem_a1);

    void *mem_a2 = stack->At(stack_offsets[2]);

    A2 a2 = *((typename CallArgTrait<A2>::CastType) mem_a2);

    void *mem_a3 = stack->At(stack_offsets[3]);

    A3 a3 = *((typename CallArgTrait<A3>::CastType) mem_a3);

    void *mem_a4 = stack->At(stack_offsets[4]);

    A4 a4 = *((typename CallArgTrait<A4>::CastType) mem_a4);
      (the_node->*(this_spec->member_fn_))(a1, a2, a3, a4);
  }

  MemberFn const member_fn_;
  static size_t const t_sizes_[4+1];
  static TypeInfo const t_types_[4+1];
};

template <class T, typename A1, typename A2, typename A3, typename A4>
size_t const GenericCallDesc<T, void(T::*)(A1, A2, A3,
    A4)>::t_sizes_[4+1] = { 0,sizeof(A1), sizeof(A2), sizeof(A3), sizeof(A4) };

template <class T, typename A1, typename A2, typename A3, typename A4>
TypeInfo const GenericCallDesc<T, void(T::*)(A1, A2, A3, A4)>::t_types_[4+1] = {
  TypeInfoOf<EmptyType>(),
      TypeInfoOf<typename detail::CallArgTrait<A1>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A2>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A3>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A4>::PlainType>()};
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename A1, typename A2, typename A3, typename A4,
    typename A5>
struct GenericCallDesc<T, void(T::*)(A1, A2, A3, A4, A5)> : public CallDesc {
  typedef GenericCallDesc<T, void(T::*)(A1, A2, A3, A4, A5)> ThisSpec;
  typedef void(T::*MemberFn)(A1, A2, A3, A4, A5);

  GenericCallDesc(MemberFn member_fn, char const *signature)
      : CallDesc(signature, t_sizes_, t_types_, 5+1, &Exec),
          member_fn_(member_fn) {}

  static void Exec(CallDesc const *spec, VStack *stack, void *node,
                   size_t *stack_offsets) {
    ThisSpec *this_spec = (ThisSpec *)spec;
    using namespace detail;
    T *the_node = (T *) node;
    void *mem_a1 = stack->At(stack_offsets[1]);

    A1 a1 = *((typename CallArgTrait<A1>::CastType) mem_a1);

    void *mem_a2 = stack->At(stack_offsets[2]);

    A2 a2 = *((typename CallArgTrait<A2>::CastType) mem_a2);

    void *mem_a3 = stack->At(stack_offsets[3]);

    A3 a3 = *((typename CallArgTrait<A3>::CastType) mem_a3);

    void *mem_a4 = stack->At(stack_offsets[4]);

    A4 a4 = *((typename CallArgTrait<A4>::CastType) mem_a4);

    void *mem_a5 = stack->At(stack_offsets[5]);

    A5 a5 = *((typename CallArgTrait<A5>::CastType) mem_a5);
      (the_node->*(this_spec->member_fn_))(a1, a2, a3, a4, a5);
  }

  MemberFn const member_fn_;
  static size_t const t_sizes_[5+1];
  static TypeInfo const t_types_[5+1];
};

template <class T, typename A1, typename A2, typename A3, typename A4,
    typename A5>
size_t const GenericCallDesc<T, void(T::*)(A1, A2, A3, A4,
    A5)>::t_sizes_[5+1] = { 0,sizeof(A1), sizeof(A2), sizeof(A3), sizeof(A4),
    sizeof(A5) };

template <class T, typename A1, typename A2, typename A3, typename A4,
    typename A5>
TypeInfo const GenericCallDesc<T, void(T::*)(A1, A2, A3, A4,
    A5)>::t_types_[5+1] = {
  TypeInfoOf<EmptyType>(),
      TypeInfoOf<typename detail::CallArgTrait<A1>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A2>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A3>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A4>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A5>::PlainType>()};
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
struct GenericCallDesc<T, void(T::*)(A1, A2, A3, A4, A5,
    A6)> : public CallDesc {
  typedef GenericCallDesc<T, void(T::*)(A1, A2, A3, A4, A5, A6)> ThisSpec;
  typedef void(T::*MemberFn)(A1, A2, A3, A4, A5, A6);

  GenericCallDesc(MemberFn member_fn, char const *signature)
      : CallDesc(signature, t_sizes_, t_types_, 6+1, &Exec),
          member_fn_(member_fn) {}

  static void Exec(CallDesc const *spec, VStack *stack, void *node,
                   size_t *stack_offsets) {
    ThisSpec *this_spec = (ThisSpec *)spec;
    using namespace detail;
    T *the_node = (T *) node;
    void *mem_a1 = stack->At(stack_offsets[1]);

    A1 a1 = *((typename CallArgTrait<A1>::CastType) mem_a1);

    void *mem_a2 = stack->At(stack_offsets[2]);

    A2 a2 = *((typename CallArgTrait<A2>::CastType) mem_a2);

    void *mem_a3 = stack->At(stack_offsets[3]);

    A3 a3 = *((typename CallArgTrait<A3>::CastType) mem_a3);

    void *mem_a4 = stack->At(stack_offsets[4]);

    A4 a4 = *((typename CallArgTrait<A4>::CastType) mem_a4);

    void *mem_a5 = stack->At(stack_offsets[5]);

    A5 a5 = *((typename CallArgTrait<A5>::CastType) mem_a5);

    void *mem_a6 = stack->At(stack_offsets[6]);

    A6 a6 = *((typename CallArgTrait<A6>::CastType) mem_a6);
      (the_node->*(this_spec->member_fn_))(a1, a2, a3, a4, a5, a6);
  }

  MemberFn const member_fn_;
  static size_t const t_sizes_[6+1];
  static TypeInfo const t_types_[6+1];
};

template <class T, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
size_t const GenericCallDesc<T, void(T::*)(A1, A2, A3, A4, A5,
    A6)>::t_sizes_[6+1] = { 0,sizeof(A1), sizeof(A2), sizeof(A3), sizeof(A4),
    sizeof(A5), sizeof(A6) };

template <class T, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
TypeInfo const GenericCallDesc<T, void(T::*)(A1, A2, A3, A4, A5,
    A6)>::t_types_[6+1] = {
  TypeInfoOf<EmptyType>(),
      TypeInfoOf<typename detail::CallArgTrait<A1>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A2>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A3>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A4>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A5>::PlainType>(),
      TypeInfoOf<typename detail::CallArgTrait<A6>::PlainType>()};

} // namespace example

#endif /* __EXAMPLE_CALL_DESC_H__ */
