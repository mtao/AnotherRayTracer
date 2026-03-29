#include "art/accel/LinearAccelerator.hpp"

#include "art/Intersection.hpp"
#include "art/Ray.hpp"
#include "art/accel/intersect_primitive.hpp"

namespace art::accel {

auto LinearAccelerator::build_index() -> void {
    // No spatial index to build — linear scan needs no precomputation.
}

auto LinearAccelerator::intersect(const Ray &ray,
                                  std::optional<Intersection> &isect) const
    -> bool {
    bool any_hit = false;
    for (const auto &prim : m_primitives) {
        // intersect_primitive respects and updates ray.tMax for
        // closest-hit pruning, so order doesn't matter for correctness.
        any_hit |= intersect_primitive(prim, ray, isect);
    }
    return any_hit;
}

} // namespace art::accel
