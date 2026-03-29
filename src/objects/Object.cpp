#include "art/objects/Object.hpp"

#include "art/geometry/Geometry.hpp"

namespace art::objects {

Object::Object(const geometry::Geometry &geometry)
  : _geometry(geometry.shared_from_this()) {}
Object::~Object() = default;

} // namespace art::objects
