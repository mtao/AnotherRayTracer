#pragma once
#include "art//intersection.hpp"
#include "art/geometry/bounding_box.hpp"
#include "art/point.hpp"

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
}  // namespace art::geometry
