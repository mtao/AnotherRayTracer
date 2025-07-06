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
bool Object::intersect_direct(const Ray& ray,
                              std::optional<Intersection>& isect) const {
    bool did_intersect = _geometry->intersect(ray, isect);

    // fill in material properties or whatnot
    return did_intersect;
}

// static Ptr create();

}  // namespace art::objects
