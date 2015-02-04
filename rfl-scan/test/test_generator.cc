#include "rfl/reflected.h"
#include <iostream>

using namespace rfl;

static void GenerateClass(Class *clazz) {
  for (size_t i = 0; i < clazz->GetNumClasses(); i++) {
    GenerateClass(clazz->GetClassAt(i));
  }
  for (size_t i = 0; i < clazz->GetNumProperties(); i++) {
  }
}

static void GenerateNamespace(Namespace *ns) {
  for (size_t i = 0; i < ns->GetNumNamespaces(); i++) {
    GenerateNamespace(ns->GetNamespaceAt(i));
  }
  for (size_t i = 0; i < ns->GetNumClasses(); i++) {
    Class *clazz = ns->GetClassAt(i);
    GenerateClass(clazz);
  }
}

extern "C"
int GeneratePackage(char const *path, Package *pkg) {
  std::cout << "package: " << pkg->name() << "-" << pkg->version() << std::endl;
  for (size_t i = 0; i < pkg->GetNumNamespaces(); i++) {
    GenerateNamespace(pkg->GetNamespaceAt(i));
  }
}
