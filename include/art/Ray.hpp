#pragma once
#include "art/Intersection.hpp"
#include "art/Point.hpp"
#include "art/geometry/Box.hpp"

namespace art {
namespace geometry {
class Box;
}
struct Ray {
    Point origin;
    Vector3d direction;

    bool hits_bbox(const geometry::Box& bbox, const Rational& min_t) const;
    bool hits_bbox(const geometry::Box& bbox, const Intersection& isect) const;
    bool hits_bbox(const geometry::Box& bbox,
                   const std::optional<Intersection>& isect) const;
    Point operator()(const Rational& t) const;
};
std::string format_as(const Ray& r);
}  // namespace art
