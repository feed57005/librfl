// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "example/package_manifest.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>

namespace example {

bool PackageManifest::Load(char const *filename) {
  std::ifstream is(filename, std::ios_base::in);
  std::string line;
  std::string current_section;

  if (!is.is_open())
    return false;

  while (is.good()) {
    std::getline(is, line);
    if (line.empty()) {
      // Skips the empty line.
      continue;
    }
    if (line[0] == '#' || line[0] == ';') {
      // This line is a comment.
      continue;
    }
    if (line[0] == '[') {
      // It is a section header.
      current_section = line.substr(1);
      size_t end = current_section.rfind(']');
      if (end != std::string::npos)
        current_section.erase(end);
    } else {
      std::string key, value;
      size_t key_start = 0;
      while (line[key_start] == ' ' || line[key_start] == '\t')
        key_start++;
      size_t equal = line.find('=');
      if (equal != std::string::npos) {
        size_t key_end = equal-1;
        while (key_end > 0 && (line[key_end] == ' ' || line[key_end] == '\t'))
          key_end--;
        key = line.substr(key_start, key_end-1);
        size_t value_start = equal + 1;
        while (line[value_start] == ' ' || line[value_start] == '\t')
          value_start++;
        value = line.substr(value_start);
        std::string whole_key = current_section;
        if (!current_section.empty())
          whole_key += ".";
        whole_key += key;
        entry_map_.insert(std::make_pair(whole_key, value));
      }
    }
  }
  is.close();
  return true;
}

bool PackageManifest::Save(char const *filename) {
  std::stringstream out;
  std::vector<std::string> keys;
  for (EntryMap::const_iterator it = entry_map_.begin(); it != entry_map_.end();
       ++it) {
    keys.push_back(it->first);
  }
  std::sort(keys.begin(), keys.end());
  std::string current_section = "";
  for (std::vector<std::string>::const_reverse_iterator it = keys.rbegin(); it != keys.rend(); ++it) {
    std::string const &key = *it;
    size_t pos = key.rfind(".", key.length()-1);
    std::string const &value = entry_map_.find(key)->second;
    if (pos != std::string::npos) {
      std::string section = key.substr(0, pos);
      if (current_section.compare(section) != 0) {
        out << "[" << section << "]\n";
        current_section = section;
      }
      out << "  " << key.substr(pos+1, key.length()-1) << " = " << value << "\n";
    } else {
      out << key << " = " << value << "\n";
    }
  }

  std::ofstream file_out;
  file_out.open(filename, std::ios_base::out);
  file_out << out.str();
  file_out.close();
  return true;
}

char const *PackageManifest::GetEntry(char const *entry) const {
  EntryMap::const_iterator it = entry_map_.find(std::string(entry));
  if (it != entry_map_.end()) {
    return it->second.c_str();
  }
  return nullptr;
}

void PackageManifest::SetEntry(char const *key, char const *value) {
  EntryMap::iterator it = entry_map_.find(std::string(key));
  if (it != entry_map_.end()) {
    it->second = std::string(value);
  } else {
    entry_map_.insert(std::make_pair(std::string(key), std::string(value)));
  }
}

} // namespace example
