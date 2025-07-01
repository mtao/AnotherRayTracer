#include "art/geometry/Box.hpp"

#include <array>
#include <zipper/views/nullary/UnitView.hpp>

#include "art/Ray.hpp"
namespace art::geometry {

std::string format_as(const Box& r) {
    return fmt::format("Box[{}:{}]", r.min(), r.max());
}

Box Box::bounding_box() const { return *this; }
bool Box::intersect(const Ray& ray, std::optional<Intersection>& isect) const {
    Rational max_t = isect.has_value()
                         ? isect->t
                         : Rational(std::numeric_limits<double>::max());
    const Point p = ray.origin;
    if (contains(p)) {
        return true;
    } else {
        auto check = [&](zipper::rank_type a, zipper::rank_type b,
                         zipper::rank_type c) -> bool {
            const auto o = p(a);
            Rational target;
            bool from_above = ray.direction(a) < 0;
            if (from_above) {
                target = max()(a);
                // if the target plane  requires going forward (direction is
                // negative)
                if (target > o) {
                    return false;
                }
            } else {
                target = min()(a);
                // if the target plane requires going back (direction is
                // positive)
                if (target < o) {
                    return false;
                }
            }
            Rational t = (target - ray.origin(a)) / ray.direction(a);

            if (t > max_t) {
                return false;
            } else {
                Rational bp = ray.direction(b) * t + ray.origin(b);
                Rational cp = ray.direction(c) * t + ray.origin(c);

                if (min()(b) >= bp || max()(b) <= bp) {
                    return false;
                }
                if (min()(c) >= cp || max()(c) <= cp) {
                    return false;
                }

                isect = Intersection();
                isect->t = t;
                switch (a) {
                    case 0: {
                        assert(b == 1);
                        assert(c == 2);
                        isect->position = Point(target, bp, cp);
                    } break;
                    case 1: {
                        assert(b == 0);
                        assert(c == 2);
                        isect->position = Point(bp, target, cp);
                    } break;
                    case 2: {
                        assert(b == 1);
                        assert(c == 0);
                        isect->position = Point(cp, bp, target);
                    } break;
                    default:
                        assert(false);
                }
                if (from_above) {
                    isect->normal =
                        zipper::views::nullary::unit_vector<double, 3>(a);
                } else {
                    isect->normal = -zipper::VectorBase(
                        zipper::views::nullary::unit_vector<double, 3>(a));
                }
                return true;
            }
        };

        const bool c0 = check(0, 1, 2);
        const bool c1 = check(1, 0, 2);
        const bool c2 = check(2, 1, 0);
        return c0 || c1 || c2;
    }
    return true;
}
bool Box::intersect(const Ray& ray) const {
    std::optional<Intersection> isec;
    return intersect(ray, isec);
}

Box& Box::expand(const Box& bb) {
    expand(bb._min);
    expand(bb._max);
    return *this;
}
Box& Box::expand(const Point& p) {
    std::array<Rational, 3> nmin;
    std::array<Rational, 3> nmax;
    for (size_t i = 0; i < 3; ++i) {
        nmin[i] = ::art::min(p(i), this->_min(i));
        nmax[i] = ::art::max(p(i), this->_max(i));
    }
    _min = Point(nmin[0], nmin[1], nmin[2]);
    _max = Point(nmax[0], nmax[1], nmax[2]);
    return *this;
}

bool Box::contains(const Point& p) const {
    bool above_min = (p.denominator() * _min.numerator().as_array() <
                      _min.denominator() * p.numerator().as_array())
                         .all();
    bool below_max = (p.denominator() * _max.numerator().as_array() >
                      _max.denominator() * p.numerator().as_array())
                         .all();
    return above_min && below_max;
}
bool Box::contains(const Box& bb) const {
    return contains(bb.min()) && contains(bb.max());
}
}  // namespace art::geometry
