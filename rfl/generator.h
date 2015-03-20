#ifndef __RFL_GENERATOR_H__
#define __RFL_GENERATOR_H__

#include "rfl/reflected.h"

#include <string>

namespace rfl {

class RFL_EXPORT Generator {
public:
  virtual ~Generator() {}

  void set_output_path(char const *out_path) {
    output_path_ = out_path;
  }

  virtual int Generate(Package const *pkg);

protected:
  virtual int TraverseNamespace(Namespace const *ns);
  virtual int TraverseClass(Class const *klass);

  virtual int BeginPackage(Package const *pkg) = 0;
  virtual int EndPackage(Package const *pkg) = 0;
  virtual int BeginClass(Class const *klass) = 0;
  virtual int EndClass(Class const *klass) = 0;
  virtual int BeginNamespace(Namespace const *ns) = 0;
  virtual int EndNamespace(Namespace const *ns) = 0;

protected:
  std::string output_path_;
};

} // namespace rfl

#endif /* __RFL_GENERATOR_H__ */
