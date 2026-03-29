#pragma once
#include <quiver/spatial/BVH.hpp>

#include "art/accel/SceneAccelerator.hpp"

namespace art::accel {

/// BVH-accelerated scene-level ray intersection using quiver's AABB BVH.
///
/// Builds a quiver BVH over world-space AABBs of all scene primitives.
/// Uses query_ray_nearest() for closest-hit traversal with early
/// termination.
class ART_API BVHAccelerator : public SceneAccelerator {
  public:
    auto intersect(const Ray &ray, std::optional<Intersection> &isect) const
        -> bool override;

  protected:
    auto build_index() -> void override;

  private:
    quiver::spatial::AABB_BVH<3> m_bvh;
};

} // namespace art::accel
