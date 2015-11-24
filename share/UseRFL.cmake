# Copyright (c) 2015 Pavel Novy. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set (LIBRFL_PATH ${CMAKE_CURRENT_LIST_DIR}/../..)
set (RFL_VERBOSE 0 CACHE INTERNAL "rfl-scan verbositity level" FORCE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR})
include (RFLMacros)
