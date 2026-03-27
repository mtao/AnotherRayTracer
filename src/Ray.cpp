#include "art/Ray.hpp"

#include <format>

namespace art {

std::string format_as(const Ray& r) {
    auto d = r.direction;
    return std::format("Ray[{}+t({},{},{})]", std::string(r.origin), d(0), d(1), d(2));
}
Point Ray::operator()(const Rational& t) const {
    return origin + Point(t.numerator * direction, t.denominator);
}
}  // namespace art
