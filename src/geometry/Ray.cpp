#include "art/geometry/Ray.hpp"
namespace art::geometry {
Point Ray::operator()(const Rational& t) const {
    return origin + Point(t.numerator * direction, t.denominator);
}
bool Ray::hits_bbox(const BoundingBox& bbox, const Rational& min_t) const {
    return true;
}
bool Ray::hits_bbox(const BoundingBox& bbox, const Intersection& isect) const {
    return hits_bbox(bbox, isect.t);
}
bool Ray::hits_bbox(const BoundingBox& bbox,
                    const std::optional<Intersection>& isect) const {
    if (isect) {
        return hits_bbox(bbox, *isect);
    } else {
        return hits_bbox(bbox, 0.0);
    }
}
}  // namespace art::geometry
