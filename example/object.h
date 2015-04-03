#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "rfl/types.h"
#include "rfl/any_var.h"
#include "rfl/native_library.h"

#include <string>
#include <vector>
#include <map>

namespace test {

typedef rfl::uint32 ClassId;
class ObjectClass;
class Property;
typedef std::map<std::string, Property *> PropertyMap;

struct ClassInstance {
  ClassInstance()
      : class_id_(-1),
        object_class_(nullptr),
        parent_class_instance_(nullptr),
        num_instances_(0),
        num_children_(0) {}

  ClassId class_id_;
  ObjectClass *object_class_;
  ClassInstance *parent_class_instance_;
  int num_instances_;
  int num_children_;
  PropertyMap properties_;

  void AddProperty(Property *prop);
  Property *GetProperty(char const *name) const;
};

typedef std::map<std::string, ClassInstance*> ClassNameMap;
typedef std::map<ClassId, ClassInstance *> ClassInstanceMap;

class ClassRepository {
public:
  static ClassRepository *GetSharedInstance();

  bool LoadPackage(char const *path, char const *pkgname);

  bool RegisterClass(ObjectClass *klass);
  bool UnregisterClass(ObjectClass *klass);

  ObjectClass *GetClassByName(char const *name);

private:
  ClassInstanceMap instance_map_;
  ClassNameMap name_map_;
  struct LoadedPackage {
    std::string package_name_;
    rfl::NativeLibrary native_lib_;
    LoadedPackage(char const *name, rfl::NativeLibrary lib)
        : package_name_(name), native_lib_(lib) {}
  };
  std::vector<LoadedPackage> package_libs_;
};

class Object {
public:
  virtual ~Object() {}

  ObjectClass *object_class() const { return object_class_; }

protected:
  friend class ObjectClass;
  virtual bool AttachClass(ObjectClass *klass);

private:
  ObjectClass *object_class_;
};

class Property {
public:
  Property(char const *name,
           char const *human_name,
           rfl::uint32 offset,
           ClassId cid,
           rfl::AnyVar const &default_value);

  char const *name() const { return name_; }
  char const *human_name() const { return human_name_; }
  rfl::uint32 offset() const { return offset_; }
  ClassId class_id() const { return class_id_; }
  rfl::AnyVar const &default_value() const { return default_value_; }

  template <typename T>
  void Set(Object *obj, T const &new_value, bool check = true) const {
    T *value = (T *)((char *)obj + (size_t)offset_ / 8);
    T tmp = *value;
    *value = new_value;
    if (check && !validate(obj)) {
      *value = tmp;
    }
  }

  template <typename T>
  T const &Get(Object *obj) const {
    T const *value = (T *)((char *)obj + (size_t)offset_ / 8);
    return *value;
  }

protected:
  virtual bool validate(Object *) const {
    return true;
  }

  char const *name_;
  char const *human_name_;
  rfl::uint32 offset_;
  ClassId class_id_;
  rfl::AnyVar default_value_;
};

template <typename T>
class NumericProperty : public Property {
public:
  NumericProperty(char const *name,
                  char const *human_name,
                  rfl::uint32 offset,
                  ClassId cid,
                  rfl::AnyVar const &def_value,
                  T min,
                  T max,
                  T step,
                  T page_step,
                  T page_size,
                  int precision)
      : Property(name, human_name, offset, cid, def_value),
        min_(min),
        max_(max),
        step_(step),
        page_step_(page_step),
        page_size_(page_size),
        precision_(precision)
  {}

 T min() const { return min_; }
 T max() const { return max_; }
 T step() const { return step_; }
 T page_step() const { return page_step_; }
 T page_size() const { return page_size_; }
 int precision() const { return precision_; }

protected:
  virtual bool validate(Object *obj) const {
    T const &value = Get<T>(obj);
    if (value >= min_ && value < max_)
      return true;
    return false;
  }

  T const min_;
  T const max_;
  T const step_;
  T const page_step_;
  T const page_size_;
  int const precision_;
};

class ObjectClass {
public:
  static ClassId ID;

  ObjectClass(char const *name, ClassId parent);
  virtual ~ObjectClass() {}

  virtual Object *CreateInstance();
  virtual void ReleaseInstance(Object *instance);

  ClassId class_id() const { return class_instance_->class_id_; }
  ClassId parent_class_id() const { return parent_class_id_; }
  char const *class_name() const { return class_name_; }

  Property *FindProperty(char const *name) const;

  template <class T>
  void EnumerateProperties(T const &enumerator) {
    ClassInstance *inst = class_instance_;
    while (inst) {
      for (std::pair<std::string, Property *> const &prop_pair :
           inst->properties_) {
        enumerator(prop_pair.second);
      }
      inst = inst->parent_class_instance_;
    }
  }

protected:
  virtual bool InitInstance(Object *instance);

  friend class ClassRepository;
  virtual bool InitClassInstance(ClassInstance *instance);
  virtual bool InitClassProperties(ClassInstance *instance);

private:
  ClassInstance *class_instance_;
  ClassId parent_class_id_;
  char const *class_name_;
};

typedef bool (*RegisterPackage)();

} // namespace test

#endif /* __OBJECT_H__ */
