#include "art/Rational.hpp"

#include <fmt/format.h>

#include <cmath>
namespace art {

Rational::operator std::string() const {
    return fmt::format("R[{}/{}]", numerator, denominator);
}

std::string format_as(const Rational& a) { return std::string(a); }
}  // namespace art
#if defined(ART_REDUCE_INLINING)
#include "Rational.hxx"
#endif
