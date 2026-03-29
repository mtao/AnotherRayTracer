#pragma once
#include "art/export.hpp"
#include "art/geometry/Geometry.hpp"

namespace art::geometry {
// Infinite plane at z=0 in local space.
// Transform via SceneNode to position/orient in the scene.
class ART_API Plane : public Geometry {
  public:
    auto bounding_box() const -> Box override;
    auto intersect(const Ray &ray, std::optional<Intersection> &isect) const
        -> bool override;
};
} // namespace art::geometry
