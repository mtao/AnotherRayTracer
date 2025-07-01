#pragma once
#include "art/geometry/Geometry.hpp"

namespace art::geometry {
// A unit sphere stored at the origin
class Sphere : public Geometry {
   public:
    Box bounding_box() const override;
    bool intersect(const Ray& ray,
                   std::optional<Intersection>& isect) const override;

   private:
    Rational radius = 1.;
};
}  // namespace art::geometry
