// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __RFL_GENERATOR_UTIL_H__
#define __RFL_GENERATOR_UTIL_H__

#include "rfl/rfl_export.h"
#include "rfl/generator.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <errno.h>

namespace rfl {

std::string HeaderGuard(std::string const &name, bool begin) {
  std::stringstream hguard;
  std::string id = name;
  std::replace(id.begin(), id.end(), '.', '_');
  std::replace(id.begin(), id.end(), '/', '_');
  std::replace(id.begin(), id.end(), ' ', '_');
  std::transform(id.begin(), id.end(), id.begin(), ::toupper);
  if (begin) {
    hguard << "#ifndef __" << id << "_RFL_H__\n"
           << "#define __" << id << "_RFL_H__\n\n";
  } else {
    hguard << "#endif // __" << id << "_RFL_H_\n";
  }
  return hguard.str();
}

bool WriteStreamToFile(std::string const &file,
                       std::stringstream const &content) {
  return Generator::WriteToFile(file.c_str(), content.str().c_str());
}

}  // namespace rfl

#endif /* __RFL_GENERATOR_UTIL_H__ */
