#include "art/Ray.hpp"

#include <cmath>
#include <format>

namespace art {

auto format_as(const Ray &r) -> std::string {
    auto d = r.direction;
    double tm = double(r.tMax);
    if (std::isinf(tm)) {
        return std::format(
            "Ray[{}+t({},{},{})]", std::string(r.origin), d(0), d(1), d(2));
    }
    return std::format("Ray[{}+t({},{},{}) tMax={}]",
                       std::string(r.origin),
                       d(0),
                       d(1),
                       d(2),
                       tm);
}
auto Ray::operator()(const Rational &t) const -> Point {
    return origin + Point(t.numerator * direction, t.denominator);
}
} // namespace art
