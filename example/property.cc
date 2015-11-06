#include "example/property.h"

namespace example {

PropertySpec::PropertySpec(char const *name,
                           char const *human_name,
                           TypeId cid,
                           AnyVar const &default_value)
    : name_(name),
      human_name_(human_name),
      class_id_(cid),
      default_value_(default_value) {}

////////////////////////////////////////////////////////////////////////////////

Property::Property() : property_spec_(nullptr), offset_(-1) {}

Property::Property(PropertySpec *spec, uint32 offset)
    : property_spec_(spec), offset_(offset) {}

Property::Property(Property const &x)
    : property_spec_(x.property_spec_), offset_(x.offset_) {}

Property &Property::operator=(Property const &x) {
  property_spec_ = x.property_spec_;
  offset_ = x.offset_;
  return *this;
}

} // namespace example
