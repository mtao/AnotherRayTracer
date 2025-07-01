#include "art/objects/Object.hpp"

#include "art/Ray.hpp"
#include "art/geometry/Geometry.hpp"
namespace art::objects {

Object::Object(const geometry::Geometry& geometry)
    : _geometry(geometry.shared_from_this()) {}
Object::~Object() = default;

void Object::update_bounding_box() {
    set_bounding_box(_geometry->bounding_box());
}
bool Object::intersect(const Ray& ray,
                       std::optional<Intersection>& isect) const {
    if (intersects_bounding_box(ray)) {
        if (_geometry->intersect(ray, isect)) {
            // TODO: fill in materials
            return true;
        }
    }
    return false;
}

// static Ptr create();

}  // namespace art::objects
