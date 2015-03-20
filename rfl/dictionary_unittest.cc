#include "gtest/gtest.h"
#include "rfl/dictionary.h"

namespace rfl {

TEST(DictionaryTest, Basic) {
  Dictionary rot;
  Dictionary nested;
  nested.Set("int", AnyVar(int(10)));
  nested.Set("float", AnyVar(float(100)));
  nested.Set("string", AnyVar(std::string("hello")));
  rot.Set("primitives", AnyVar(nested));

  AnyVar const &str_value = rot.GetKey(std::string("primitives.string"));
  std::cout << str_value.GetType().GetName() << std::endl;
  EXPECT_TRUE(str_value.GetType() == TypeInfoOf<std::string>());
}

TEST(DictionaryTest, MapCopy) {
  typedef std::map<std::string, std::string> StringMap;

  StringMap map1;
  map1.insert(std::make_pair(std::string("key"), std::string("value")));

  StringMap map2 = map1;
  EXPECT_TRUE(map2.find(std::string("key")) != map2.end());
}

} // namespace rfl
