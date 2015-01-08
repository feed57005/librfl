#ifndef __RFL_REFLECTED_IO_H__
#define __RFL_REFLECTED_IO_H__

#include "rfl/rfl_export.h"
#include "rfl/reflected.h"

#include <iostream>

namespace rfl {

void DumpNamespace(std::ostream &out, Namespace const *ns, int level = 0);
void DumpPackage(std::ostream &out, Package const *pkg);
void GeneratePackage(char const *path, Package const *pkg);

} // namespace rfl

#endif /* __RFL_REFLECTED_IO_H__ */
