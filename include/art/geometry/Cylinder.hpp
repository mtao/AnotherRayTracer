#pragma once
#include "art/export.hpp"
#include "art/geometry/Geometry.hpp"

namespace art::geometry {
// Unit cylinder along z-axis, z in [0,1], radius 1 in local space.
// No end caps — only the lateral surface.
// Transform via SceneNode to position/orient/scale in the scene.
class ART_API Cylinder : public Geometry {
  public:
    auto bounding_box() const -> Box override;
    auto intersect(const Ray &ray, std::optional<Intersection> &isect) const
        -> bool override;
};
} // namespace art::geometry
