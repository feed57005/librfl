librfl
------

(WIP)

Library for generating code from C++ annotations using [Clang Tooling](http://clang.llvm.org/docs/LibTooling.html).

```C++
class rfl_class(name="Hello World") HelloWorld {
public:
  rfl_method(name = "Do Something")
  void DoSomething(rfl_arg(name = "A", kind = "in") int a,
                   rfl_arg(name = "B", kind = "in") int b,);
private:
  rfl_property(id = "int_value",
               kind = "number",
               name = "Integer Value",
               default = 10,
               min = 0,
               max = 100,
               step = 1,
               page_step = 1,
               page_size = 1,
               precision = 0)
  int int_value_;
};
```

Currenly supports annotating classes, fields, methods and enumerations. See `example` directory for usage.

### Installation
- Requires Clang 3.6 libraries and CMake to be installed
- Set environment variable `LLVM_PATH` to point to the Clang installation (eg. `export LLVM_PATH=/opt/clang`)
- Use CMake to generate the build

### Known limitations
- Currenly only single inheritance is supported
- Member field offsets may differ when using other compiler than Clang
- Tested on Linux and Mac OS X
