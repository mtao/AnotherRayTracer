#include "art/Ray.hpp"
namespace art {

std::string format_as(const Ray& r) {
    return fmt::format("Ray[{}+t{}]", r.origin, r.direction);
}
Point Ray::operator()(const Rational& t) const {
    return origin + Point(t.numerator * direction, t.denominator);
}
}  // namespace art
