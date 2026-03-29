#include "art/accel/intersect_primitive.hpp"

#include "art/Ray.hpp"
#include "art/geometry/Geometry.hpp"

namespace art::accel {

auto intersect_primitive(const ScenePrimitive &prim,
                         const Ray &ray,
                         std::optional<Intersection> &isect) -> bool {
    auto inv = prim.world_transform.inverse();

    // Transform ray to local space
    Ray local_ray;
    local_ray.origin =
        Point((inv.to_matrix() * ray.origin.homogeneous()).eval());
    local_ray.direction = inv.linear() * ray.direction;
    local_ray.tMax = ray.tMax;

    if (prim.geometry->intersect(local_ray, isect)) {
        // Transform intersection back to world space
        if (isect) {
            auto M = prim.world_transform.to_matrix();
            // Position: world = M * local_pos
            isect->position = Point((M * isect->position.homogeneous()).eval());

            // Normal transforms by inverse-transpose of the linear part
            auto inv_t = inv.linear().transpose();
            isect->geometric_normal = inv_t * isect->geometric_normal;

            // Tangent vectors transform by the linear part directly
            auto L = prim.world_transform.linear();
            isect->dpdu = L * isect->dpdu;
            isect->dpdv = L * isect->dpdv;
        }
        // Propagate tMax back to the caller's ray
        ray.tMax = local_ray.tMax;
        return true;
    }
    return false;
}

} // namespace art::accel
