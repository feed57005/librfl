// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __LIBRFL_ANNOTATIONS_H__
#define __LIBRFL_ANNOTATIONS_H__

#if defined(__RFL_SCAN__)
#define rfl_primitive(...) \
  __attribute__((annotate("primitive:{" #__VA_ARGS__ "}")))
#define rfl_class(...) __attribute__((annotate("class:{" #__VA_ARGS__ "}")))
#define rfl_enum(...) __attribute__((annotate("enum:{" #__VA_ARGS__ "}")))
#define rfl_property(...) \
  __attribute__((annotate("property:{" #__VA_ARGS__ "}")))
#define rfl_field(...) __attribute__((annotate("field:{" #__VA_ARGS__ "}")))
#define rfl_method(...) __attribute__((annotate("method:{" #__VA_ARGS__ "}")))
#define rfl_arg(...) __attribute__((annotate("arg:{" #__VA_ARGS__ "}")))
#else
#define rfl_primitive(...)
#define rfl_class(...)
#define rfl_enum(...)
#define rfl_property(...)
#define rfl_field(...)
#define rfl_method(...)
#define rfl_arg(...)
#endif

#endif /* __LIBRFL_ANNOTATIONS_H__ */
