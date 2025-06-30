#include "art/geometry/Ray.hpp"
namespace art::geometry {
Point Ray::operator()(const Rational& t) const {
    return origin + Point(t.numerator * direction, t.denominator);
}
bool Ray::hits_bbox(const BoundingBox& bbox, const Rational& min_t) const {
    const Point p = (*this)(min_t);
    if (bbox.contains(p)) {
        return true;
    } else {
        auto check = [&](zipper::rank_type a, zipper::rank_type b,
                         zipper::rank_type c) -> bool {
            const auto o = p(a);
            Rational target;
            if (direction(a) < 0) {
                target = bbox.max()(a);
                if (target < o) {
                    return false;
                }
            } else {
                target = bbox.min()(a);
                if (target > o) {
                    return true;
                }
            }
            Rational t = (target - origin(a)) / direction(a);

            if (t < min_t) {
                return false;
            } else {
                Rational bp = direction(b) * t + origin(b);

                if (bbox.min()(b) >= bp || bbox.max()(b) >= bp) {
                    return false;
                }
                Rational cp = direction(c) * t + origin(c);
                if (bbox.min()(c) >= cp || bbox.max()(c) >= cp) {
                    return false;
                }
            }

            return true;
        };

        return check(0, 1, 2) || check(1, 0, 2) || check(2, 0, 1);
    }
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
