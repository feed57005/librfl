#include "rfl/generator.h"

namespace rfl {

int Generator::Generate(Package const *pkg) {
  int ret = BeginPackage(pkg);
  if (ret)
    return ret;

  for (size_t i = 0; i < pkg->GetNumNamespaces(); i++) {
    ret = TraverseNamespace(pkg->GetNamespaceAt(i));
    if (ret)
      return ret;
  }

  return EndPackage(pkg);
}

int Generator::TraverseNamespace(Namespace const *ns) {
  int ret = BeginNamespace(ns);
  for (size_t i = 0; i < ns->GetNumNamespaces(); i++) {
    ret = TraverseNamespace(ns->GetNamespaceAt(i));
    if (ret)
      return ret;
  }
  for (size_t i = 0; i < ns->GetNumClasses(); i++) {
    Class *clazz = ns->GetClassAt(i);
    ret = TraverseClass(clazz);
    if (ret)
      return ret;
  }
  ret = EndNamespace(ns);
  return ret;
}

int Generator::TraverseClass(Class const *klass) {
  int ret = BeginClass(klass);
  for (size_t i = 0; i < klass->GetNumClasses(); i++) {
    ret = TraverseClass(klass->GetClassAt(i));
    if (ret)
      return ret;
  }
  return EndClass(klass);
}

} // namespace rfl
