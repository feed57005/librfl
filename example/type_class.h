#ifndef __EXAMPLE_TYPE_CLASS_H__
#define __EXAMPLE_TYPE_CLASS_H__

#include "example/example_export.h"
#include "rfl/types.h"

namespace example {

typedef rfl::uint32 TypeId;

class Package;
class TypeClass;

class EXAMPLE_EXPORT TypeClassInstance {
public:
  TypeClassInstance(TypeId id, TypeClass *type_class)
      : type_id_(id), type_class_(type_class) {}

  virtual ~TypeClassInstance() {}

  TypeId type_id() const {
    return type_id_;
  }

  TypeClass *type_class() const {
    return type_class_;
  }

protected:
  TypeId type_id_;
  TypeClass *type_class_;
};

class EXAMPLE_EXPORT TypeClass {
public:
  enum Kind { ENUM_KIND = 0, OBJECT_KIND };

  TypeClass(char const *type_name)
      : type_class_instance_(nullptr), type_name_(type_name), package_(nullptr) {}

  virtual ~TypeClass() {}

  TypeId type_id() const { return type_class_instance_->type_id(); }
  char const *type_name() const { return type_name_; }
  Package *package() const { return package_; }

  virtual Kind GetTypeClassKind() const = 0;

protected:
  friend class TypeRepository;
  TypeClassInstance *type_class_instance_;
  char const *type_name_;
  Package *package_;
};

} // namespace example

#endif /* __EXAMPLE_TYPE_CLASS_H__ */
