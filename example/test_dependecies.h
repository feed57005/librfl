#ifndef __TEST_DEPENDECIES_H__
#define __TEST_DEPENDECIES_H__

#include "test_annotations.h"

namespace test {

class rfl_class(name = "Extended Test Object", package="test_dependencies") ExtendedTestObject
    : public TestObject {

  void blabla();

private:
  rfl_property(id = "count",
               kind = "number",
               name = "Count",
               default = 10,
               min = 0,
               max = 100,
               step = 1,
               page_step = 1,
               page_size = 1,
               precision = 0) int count_;
};

} // namespace test

#endif /* __TEST_DEPENDECIES_H__ */
