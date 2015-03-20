#ifndef __RFL_DICTIONARY_H__
#define __RFL_DICTIONARY_H__

#include "rfl/type_info.h"
#include "rfl/any_var.h"

#include <string>
#include <map>
#include <sstream>
#include <string>

#include <iostream>

namespace rfl {

class Dictionary {
public:
  Dictionary() {}
  ~Dictionary() {}

  Dictionary(Dictionary const &dict) : dict_(dict.dict_) {}

  Dictionary &operator()(Dictionary const &x) {
    dict_ = x.dict_;
    return *this;
  }

  bool operator==(Dictionary const &x) const {
    return false;
  }

  AnyVar const &GetKey(std::string const &key) const {
    Dictionary const *dict = this;
    std::stringstream ss(key);
    std::string item;
    AnyVar const *value = &AnyVar::empty();
    while (std::getline(ss, item, '.')) {
      value = &dict->Get(item);
      if (value->GetType() == TypeInfoOf<EmptyType>())
        return AnyVar::empty();
      if (value->GetType() == TypeInfoOf<Dictionary>()) {
        dict = &value->Cast<Dictionary>();
      }
    }
    return *value;
  }

  AnyVar const &Get(std::string const &key) const {
    DictMap::const_iterator it = dict_.find(key);
    if (it != dict_.end()) {
      return it->second;
    }
    return AnyVar::empty();
  }

  void Set(std::string const &key, AnyVar const &value) {
    dict_.insert(std::make_pair(key,value));
  }

private:
  typedef std::map<std::string, AnyVar> DictMap;
  DictMap dict_;
};

} // namespace rfl

RFL_NAME_TYPE_0(RFL_EXPORT, "Dictionary", Dictionary)

#endif /* __RFL_DICTIONARY_H__ */
