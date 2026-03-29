#include "art/accel/BVHAccelerator.hpp"

#include <array>

#include "art/Intersection.hpp"
#include "art/Ray.hpp"
#include "art/accel/intersect_primitive.hpp"
#include "art/geometry/Geometry.hpp"

namespace art::accel {

auto BVHAccelerator::build_index() -> void {
    if (m_primitives.empty()) { return; }

    // Compute world-space AABBs for each primitive by transforming
    // the 8 corners of the local bounding box through the world transform.
    using aabb_type = quiver::spatial::AABB<3>;
    std::vector<aabb_type> bounds(m_primitives.size());

    for (size_t i = 0; i < m_primitives.size(); ++i) {
        const auto &prim = m_primitives[i];
        auto local_bb = prim.geometry->bounding_box();

        Vector3d lo = local_bb.min();
        Vector3d hi = local_bb.max();

        auto M = prim.world_transform.to_matrix();

        // Enumerate all 8 corners of the local AABB, transform to world
        // space, and build a world-space AABB that encloses them all.
        auto world_aabb = aabb_type::empty();
        for (int c = 0; c < 8; ++c) {
            Vector3d corner;
            corner(0) = (c & 1) ? hi(0) : lo(0);
            corner(1) = (c & 2) ? hi(1) : lo(1);
            corner(2) = (c & 4) ? hi(2) : lo(2);

            // Transform corner to world space via the 4x4 matrix
            Vector4d h;
            h(0) = corner(0);
            h(1) = corner(1);
            h(2) = corner(2);
            h(3) = 1.0;
            auto wh = (M * h).eval();
            // Dehomogenize
            std::array<double, 3> wp{
                wh(0) / wh(3), wh(1) / wh(3), wh(2) / wh(3)};
            world_aabb.expand(wp);
        }
        bounds[i] = world_aabb;
    }

    m_bvh.build(bounds);
}

auto BVHAccelerator::intersect(const Ray &ray,
                               std::optional<Intersection> &isect) const
    -> bool {
    if (m_primitives.empty()) { return false; }

    // Extract origin and direction as raw spans for quiver's API
    Vector3d origin_vec = ray.origin;
    std::array<double, 3> origin_arr{
        origin_vec(0), origin_vec(1), origin_vec(2)};
    std::array<double, 3> dir_arr{
        ray.direction(0), ray.direction(1), ray.direction(2)};

    auto [prim_idx, t_val] = m_bvh.query_ray_nearest(
        std::span<const double, 3>{origin_arr},
        std::span<const double, 3>{dir_arr},
        [&](uint32_t idx) -> std::optional<double> {
            // Make a copy of ray so each candidate gets the current tMax
            // but doesn't pollute the state for other candidates
            std::optional<Intersection> candidate_isect;

            // We need a temporary ray with the current tMax
            Ray candidate_ray;
            candidate_ray.origin = ray.origin;
            candidate_ray.direction = ray.direction;
            candidate_ray.tMax = ray.tMax;

            if (intersect_primitive(
                    m_primitives[idx], candidate_ray, candidate_isect)) {
                // Store the candidate result — we'll pick the closest
                double t = double(candidate_ray.tMax);
                // Update the outer ray's tMax so subsequent BVH traversal
                // can prune farther nodes
                ray.tMax = candidate_ray.tMax;
                isect = candidate_isect;
                return t;
            }
            return std::nullopt;
        });

    return prim_idx != uint32_t(-1);
}

} // namespace art::accel
