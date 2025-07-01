#pragma once
#include "art/Intersection.hpp"
#include "art/Point.hpp"
#include "art/geometry/BoundingBox.hpp"

namespace art::geometry {
struct Ray {
    Point origin;
    Vector3d direction;

    bool hits_bbox(const BoundingBox& bbox, const Rational& min_t) const;
    bool hits_bbox(const BoundingBox& bbox, const Intersection& isect) const;
    bool hits_bbox(const BoundingBox& bbox,
                   const std::optional<Intersection>& isect) const;
    Point operator()(const Rational& t) const;
};
std::string format_as(const Ray& r);
}  // namespace art::geometry
