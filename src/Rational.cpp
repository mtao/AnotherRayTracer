#include "art/Rational.hpp"

#include <fmt/format.h>

#include <cmath>
namespace art {
Rational min(const Rational& a, const Rational& b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}
Rational max(const Rational& a, const Rational& b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}
Rational clamp(const Rational& v, const Rational& lower,
               const Rational& upper) {
    if (v < lower) {
        return lower;
    } else if (v > upper) {
        return upper;
    } else {
        return v;
    }
}
Rational sqrt(const Rational& r) {
    return {std::sqrt(r.numerator), std::sqrt(r.denominator)};
}
Rational::operator std::string() const {
    return fmt::format("R[{}/{}]", numerator, denominator);
}
}  // namespace art
