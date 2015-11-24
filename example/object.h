// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __EXAMPLE_OBJECT_H__
#define __EXAMPLE_OBJECT_H__

#include "example/example_export.h"
#include "example/any_var.h"
#include "example/type_class.h"
#include "example/call_desc.h"

#include "rfl/types.h"

#include <map>

namespace example {

class ObjectClass;
class ObjectClassInstance;
class Property;
class PropertySpec;
class Method;
class TypeRepository;

typedef std::map<std::string, Property *> PropertyMap;


class EXAMPLE_EXPORT ObjectClassInstance : public TypeClassInstance {
public:
  ObjectClassInstance(TypeId id,
                      ObjectClass *klass,
                      ObjectClassInstance *parent);
  ~ObjectClassInstance() override;

  ObjectClassInstance *parent_instance() const { return parent_instance_; }

  PropertySpec *GetPropertySpec(char const *name) const;
  void AddPropertySpec(PropertySpec *prop);

  Method *GetMethod(char const *name) const;
  void AddMethod(Method *method);

  template <class T>
  void EnumerateProperties(T const &enumerator) const {
    ObjectClassInstance const *inst = this;
    while (inst) {
      for (std::pair<std::string, PropertySpec *> const &prop_pair :
           inst->properties_) {
        enumerator(prop_pair.second);
      }
      inst = inst->parent_instance_;
    }
  }

  template <class T>
  void EnumerateMethods(T const &enumerator) const {
    ObjectClassInstance const *inst = this;
    while (inst) {
      for (std::pair<std::string, Method *> const &method_pair :
           inst->methods_) {
        enumerator(method_pair.second);
      }
      inst = inst->parent_instance_;
    }
  }

protected:
  ObjectClassInstance *parent_instance_;
  typedef std::map<std::string, PropertySpec *> PropertySpecMap;
  typedef std::map<std::string, Method *> MethodMap;
  MethodMap methods_;
  PropertySpecMap properties_;
};

////////////////////////////////////////////////////////////////////////////////

class EXAMPLE_EXPORT Object {
public:
  virtual ~Object() {}

  ObjectClass *object_class() const { return object_class_; }
  PropertyMap const &properties() const { return properties_; }
  PropertyMap &properties() { return properties_; }

protected:
  friend class ObjectClass;
  virtual bool AttachClass(ObjectClass *klass);

private:
  ObjectClass *object_class_;
  PropertyMap properties_;
};

class EXAMPLE_EXPORT ObjectClass : public TypeClass {
public:
  static TypeId ID;

  ObjectClass(char const *name, TypeId parent);
  ~ObjectClass() override;

  Kind GetTypeClassKind() const override { return OBJECT_KIND; }

  virtual Object *CreateInstance();
  virtual void ReleaseInstance(Object *instance);

  TypeId class_id() const { return class_instance()->type_id(); }
  TypeId parent_class_id() const { return parent_class_id_; }
  char const *class_name() const { return class_name_; }

  PropertySpec *FindPropertySpec(char const *name) const;

  Method *FindMethod(char const *name) const;

  ObjectClassInstance *class_instance() const {
    return (ObjectClassInstance *)type_class_instance_;
  }

protected:
  friend class TypeRepository;
  virtual bool InitInstance(Object *instance);

  void InitObjectProperties(Object *obj);

protected:
  TypeId parent_class_id_;
  char const *class_name_;
};

////////////////////////////////////////////////////////////////////////////////

class EXAMPLE_EXPORT Method {
public:
  Method(char const *name, char const *human_name, TypeId cid, CallDesc *desc);

  char const *name() const { return name_; }
  char const *human_name() const { return human_name_; }
  TypeId class_id() const { return class_id_; }
  CallDesc *call_description() const { return call_desc_; }

protected:
  char const *name_;
  char const *human_name_;
  TypeId class_id_;
  CallDesc *call_desc_;
};

} // namespace example

#endif /* __EXAMPLE_OBJECT_H__ */
