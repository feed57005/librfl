// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __RFL_EXPORT_H__
#define __RFL_EXPORT_H__

#if defined(WIN32)

#if defined(RFL_IMPLEMENTATION)
#define RFL_EXPORT __declspec(dllexport)
#else
#define RFL_EXPORT __declspec(dllimport)
#endif

#else
#if defined(BLOCKIT_IMPLEMENTATION)
#define RFL_EXPORT __attribute__((visibility("default")))
#else
#define RFL_EXPORT
#endif
#endif

#endif /* __RFL_EXPORT_H__ */
