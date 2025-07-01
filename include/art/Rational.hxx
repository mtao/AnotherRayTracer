#pragma once
#include <cmath>

#include "art/Rational.hpp"
namespace art {
ART_INLINE Rational min(const Rational& a, const Rational& b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}
ART_INLINE Rational max(const Rational& a, const Rational& b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}
ART_INLINE Rational clamp(const Rational& v, const Rational& lower,
                          const Rational& upper) {
    if (v < lower) {
        return lower;
    } else if (v > upper) {
        return upper;
    } else {
        return v;
    }
}
ART_INLINE Rational sqrt(const Rational& r) {
    return {std::sqrt(r.numerator), std::sqrt(r.denominator)};
}
ART_INLINE std::weak_ordering Rational::operator<=>(const Rational& o) const {
    const bool zero = numerator == 0;
    const bool ozero = o.numerator == 0;
    if (zero && ozero) {
        return std::weak_ordering::equivalent;
    } else if (zero) {
        return o.positive()
                   //
                   ? std::weak_ordering::less
                   : std::weak_ordering::greater;
    } else if (ozero) {
        return positive() ? std::weak_ordering::greater
                          : std::weak_ordering::less;
    } else {
        const double a = numerator * o.denominator;
        const double b = o.numerator * denominator;

        if (std::signbit(denominator) ^ std::signbit(o.denominator)) {
            return std::weak_order(b, a);
        } else {
            return std::weak_order(a, b);
        }
    }
}

}  // namespace art
