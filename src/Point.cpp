#include "art/Point.hpp"

#include <format>

namespace art {
Point::operator std::string() const {
    auto n = numerator().eval();
    return std::format("P[({} {} {})/{}]", n(0), n(1), n(2), denominator());
}
}  // namespace art
