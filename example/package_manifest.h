#ifndef __PACKAGE_MANIFEST_H__
#define __PACKAGE_MANIFEST_H__

#include <map>
#include <string>

namespace test {

class PackageManifest {
public:
  bool Load(char const *filename);
  bool Save(char const *filename);
  char const *GetEntry(char const *entry) const;
  void SetEntry(char const *key, char const *value);

  template <class E>
  void Enumerate(E &enumerator) const {
    for (EntryMap::const_iterator it = entry_map_.begin();
         it != entry_map_.end(); ++it) {
      enumerator(it->first, it->second);
    }
  }
private:
  typedef std::map<std::string, std::string> EntryMap;
  EntryMap entry_map_;
};

} // namespace test

#endif /* __PACKAGE_MANIFEST_H__ */
