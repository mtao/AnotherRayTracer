#include "art/Rational.hpp"

#include <cmath>
namespace art {

Rational::operator std::string() const {
    return std::format("R[{}/{}]", numerator, denominator);
}
}  // namespace art
#if defined(ART_REDUCE_INLINING)
#include "Rational.hxx"
#endif
