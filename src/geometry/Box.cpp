#include "art/geometry/Box.hpp"

#include <array>
#include <format>
#include <zipper/expression/nullary/Unit.hpp>

#include "art/Ray.hpp"
namespace art::geometry {

auto format_as(const Box &r) -> std::string {
    return std::format(
        "Box[{}:{}]", std::string(r.min()), std::string(r.max()));
}

auto Box::bounding_box() const -> Box { return *this; }
auto Box::intersect(const Ray &ray, std::optional<Intersection> &isect) const
    -> bool {
    const Point p = ray.origin;
    if (contains(p)) {
        return true;
    } else {
        auto check = [&](zipper::rank_type a,
                         zipper::rank_type b,
                         zipper::rank_type c) -> bool {
            const auto o = p(a);
            Rational target;
            bool from_above = ray.direction(a) < 0;
            if (from_above) {
                target = max()(a);
                if (target > o) { return false; }
            } else {
                target = min()(a);
                if (target < o) { return false; }
            }
            Rational t = (target - ray.origin(a)) / ray.direction(a);

            if (t > ray.tMax) { return false; }
            // Check against existing intersection
            if (isect && isect->t < t) { return false; }

            Rational bp = ray.direction(b) * t + ray.origin(b);
            Rational cp = ray.direction(c) * t + ray.origin(c);

            if (min()(b) >= bp || max()(b) <= bp) { return false; }
            if (min()(c) >= cp || max()(c) <= cp) { return false; }

            ray.tMax = t;

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
                isect->geometric_normal =
                    zipper::expression::nullary::unit_vector<double, 3>(a);
            } else {
                isect->geometric_normal = -Vector3d(
                    zipper::expression::nullary::unit_vector<double, 3>(a));
            }

            // Face-local UV: the two non-hit axes mapped to [0,1]
            // relative to the box extents
            Vector3d hit_pos = Vector3d(isect->position);
            Vector3d box_min = Vector3d(min());
            Vector3d box_max = Vector3d(max());
            double extent_b = double(box_max(b)) - double(box_min(b));
            double extent_c = double(box_max(c)) - double(box_min(c));
            double u_val = extent_b > 0
                               ? (hit_pos(b) - double(box_min(b))) / extent_b
                               : 0.0;
            double v_val = extent_c > 0
                               ? (hit_pos(c) - double(box_min(c))) / extent_c
                               : 0.0;
            isect->uv = Vector2d{u_val, v_val};

            // dpdu/dpdv: unit vectors along the face plane
            isect->dpdu = Vector3d(
                zipper::expression::nullary::unit_vector<double, 3>(b));
            isect->dpdv = Vector3d(
                zipper::expression::nullary::unit_vector<double, 3>(c));

            isect->geometry = this;
            // Face index: 0-2 positive faces, 3-5 negative faces
            isect->primitive_index = from_above ? static_cast<uint32_t>(a)
                                                : static_cast<uint32_t>(a + 3);

            return true;
        };

        const bool c0 = check(0, 1, 2);
        const bool c1 = check(1, 0, 2);
        const bool c2 = check(2, 1, 0);
        return c0 || c1 || c2;
    }
    return true;
}

auto Box::intersect(const Ray &ray) const -> bool {
    std::optional<Intersection> isec;
    return intersect(ray, isec);
}

auto Box::expand(const Box &bb) -> Box & {
    expand(bb._min);
    expand(bb._max);
    return *this;
}
auto Box::expand(const Point &p) -> Box & {
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

auto Box::contains(const Point &p) const -> bool {
    bool above_min = (p.denominator() * _min.numerator().as_array()
                      < _min.denominator() * p.numerator().as_array())
                         .all();
    bool below_max = (p.denominator() * _max.numerator().as_array()
                      > _max.denominator() * p.numerator().as_array())
                         .all();
    return above_min && below_max;
}
auto Box::contains(const Box &bb) const -> bool {
    return contains(bb.min()) && contains(bb.max());
}
} // namespace art::geometry
