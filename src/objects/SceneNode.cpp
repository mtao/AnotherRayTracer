#include "art/objects/SceneNode.hpp"

#include "art/Ray.hpp"
#include "art/geometry/Geometry.hpp"
namespace art::objects {

bool SceneNode::intersects_bounding_box(const Ray& ray) const {
    return bounding_box().intersect(ray);
}

bool SceneNode::intersect(const Ray& ray,
                          std::optional<Intersection>& isect) const {
    if (intersects_bounding_box(ray)) {
        Ray r{(_transform.as_matrix() * ray.origin.homogeneous()).eval(),
              _transform.rotation().transpose() * ray.direction};

        return intersect_direct(r, isect);
    }
    return false;
}
void SceneNode::set_bounding_box(const geometry::Box& bounding_box) {
    _bounding_box = bounding_box;
}
// static Ptr create();

}  // namespace art::objects
