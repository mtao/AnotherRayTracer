#pragma once
#include "art/export.hpp"
#include "art/geometry/Geometry.hpp"

namespace art::geometry {
// Unit disk at z=0, radius 1 in local space.
// Transform via SceneNode to position/orient/scale in the scene.
class ART_API Disk : public Geometry {
  public:
    auto bounding_box() const -> Box override;
    auto intersect(const Ray &ray, std::optional<Intersection> &isect) const
        -> bool override;
};
} // namespace art::geometry
