#pragma once
#include "art/objects/Object.hpp"

namespace art::objects {
// A unit sphere stored at the origin
class Sphere : public Object {
   public:
    void update_bbox() override;
    bool intersect(const geometry::Ray& ray,
                   std::optional<Intersection>& isect) const override;

   private:
    Rational radius = 1.;
};
}  // namespace art::objects
