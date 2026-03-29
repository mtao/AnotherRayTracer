#include "art/objects/SceneNode.hpp"

#include "art/Ray.hpp"
#include "art/geometry/Geometry.hpp"
namespace art::objects {

auto SceneNode::intersects_bounding_box(const Ray &ray) const -> bool {
    return bounding_box().intersect(ray);
}

auto SceneNode::intersect(const Ray &ray,
                          std::optional<Intersection> &isect) const -> bool {
    if (intersects_bounding_box(ray)) {
        auto inv = _transform.inverse();
        Ray local_ray;
        local_ray.origin =
            Point((inv.to_matrix() * ray.origin.homogeneous()).eval());
        local_ray.direction = inv.linear() * ray.direction;
        local_ray.tMax = ray.tMax;

        if (intersect_direct(local_ray, isect)) {
            // Transform intersection back to parent space
            if (isect) {
                auto M = _transform.to_matrix();
                // Transform position back to world space
                isect->position =
                    Point((M * isect->position.homogeneous()).eval());

                // Normal transforms by inverse-transpose of the linear part
                // For affine transforms: (M^{-1})^T * n
                auto inv_t = inv.linear().transpose();
                isect->geometric_normal = inv_t * isect->geometric_normal;

                // Tangent vectors transform by the linear part directly
                auto L = _transform.linear();
                isect->dpdu = L * isect->dpdu;
                isect->dpdv = L * isect->dpdv;
            }
            // Propagate tMax back to parent ray
            ray.tMax = local_ray.tMax;
            return true;
        }
    }
    return false;
}
void SceneNode::set_bounding_box(const geometry::Box &bounding_box) {
    _bounding_box = bounding_box;
}

} // namespace art::objects
