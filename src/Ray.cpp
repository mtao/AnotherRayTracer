#include "art/Ray.hpp"
namespace art {

std::string format_as(const Ray& r) {
    return fmt::format("Ray[{}+t{}]", r.origin, r.direction);
}
Point Ray::operator()(const Rational& t) const {
    return origin + Point(t.numerator * direction, t.denominator);
}
bool Ray::hits_bbox(const geometry::Box& bbox, const Rational& min_t) const {
    const Point p = (*this)(min_t);
    // spdlog::info("Ray bbox check on ray {}, BBox {}, mint {}", *this, bbox,
    //              min_t);
    if (bbox.contains(p)) {
        return true;
    } else {
        auto check = [&](zipper::rank_type a, zipper::rank_type b,
                         zipper::rank_type c) -> bool {
            const auto o = p(a);
            Rational target;
            if (direction(a) < 0) {
                target = bbox.max()(a);
                // if the target plane  requires going forward (direction is
                // negative)
                if (target > o) {
                    return false;
                }
            } else {
                target = bbox.min()(a);
                // if the target plane requires going back (direction is
                // positive)
                if (target < o) {
                    return false;
                }
            }
            Rational t = (target - origin(a)) / direction(a);
            // spdlog::info("plane got a t of {}", t);

            if (t < min_t) {
                // spdlog::info("Mint fail");
                return false;
            } else {
                Rational bp = direction(b) * t + origin(b);
                Rational cp = direction(c) * t + origin(c);
                // spdlog::info("Got bc = {} {}", bp, cp);

                if (bbox.min()(b) >= bp || bbox.max()(b) <= bp) {
                    return false;
                }
                if (bbox.min()(c) >= cp || bbox.max()(c) <= cp) {
                    return false;
                }
            }

            return true;
        };

        const bool c0 = check(0, 1, 2);
        const bool c1 = check(1, 0, 2);
        const bool c2 = check(2, 1, 0);
        return c0 || c1 || c2;
    }
    return true;
}
bool Ray::hits_bbox(const geometry::Box& bbox,
                    const Intersection& isect) const {
    return hits_bbox(bbox, isect.t);
}
bool Ray::hits_bbox(const geometry::Box& bbox,
                    const std::optional<Intersection>& isect) const {
    if (isect) {
        return hits_bbox(bbox, *isect);
    } else {
        return hits_bbox(bbox, 0.0);
    }
}
}  // namespace art
