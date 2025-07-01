#if !defined(ART_GEOMETRY_GEOMETRY_HPP)
#define ART_GEOMETRY_GEOMETRY_HPP
#pragma once
#include <memory>
#include <optional>

#include "art/Intersection.hpp"

namespace art {
class Ray;
}
namespace art::geometry {
class Box;
class Geometry: public std::enable_shared_from_this<Geometry> {
   public:
    virtual geometry::Box bounding_box() const = 0;

    virtual bool intersect(const Ray& ray,
                           std::optional<Intersection>& isect) const = 0;
};
}  // namespace art::geometry
#include "Box.hpp"
#endif
