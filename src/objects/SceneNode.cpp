#include "art/objects/SceneNode.hpp"

#include "art/Ray.hpp"
#include "art/geometry/Geometry.hpp"
namespace art::objects {

bool SceneNode::intersects_bounding_box(const Ray& ray) const {
    return bounding_box().intersect(ray);
}

bool SceneNode::transform_and_intersect(
    const Ray& ray, std::optional<Intersection>& isect) const {
     Ray r{(_transform.as_matrix() * ray.origin.homogeneous()).eval(),
          _transform.rotation().transpose() * ray.direction};

    return intersect(r, isect);
}
// static Ptr create();

}  // namespace art::objects
