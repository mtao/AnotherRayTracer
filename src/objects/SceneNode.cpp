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
        auto inv = _transform.inverse();
        Ray r{(inv.to_matrix() * ray.origin.homogeneous()).eval(),
              inv.linear() * ray.direction};

        return intersect_direct(r, isect);
    }
    return false;
}
void SceneNode::set_bounding_box(const geometry::Box& bounding_box) {
    _bounding_box = bounding_box;
}

}  // namespace art::objects
