#include "art/point.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <zipper/views/nullary/ConstantView.hpp>

namespace art {
Point Point::Constant(const Rational& r) {
    Point p;
    p.numerator() = zipper::views::nullary::ConstantView(r.numerator);
    p.denominator() = r.denominator;
    return p;
}
Point::Point(const Rational& a, const Rational& b, const Rational& c) {
    auto n = numerator();
    auto& d = denominator();
    if (a.denominator == b.denominator && a.denominator == c.denominator) {
        n(0) = a.numerator;
        n(1) = b.numerator;
        n(2) = c.numerator;
        d = a.denominator;
    } else if (a.denominator == b.denominator) {
        n(0) = a.numerator * c.denominator;
        n(1) = b.numerator * c.denominator;
        n(2) = c.numerator * a.denominator;
        d = a.denominator * c.denominator;
    } else if (a.denominator == c.denominator) {
        n(0) = a.numerator * b.denominator;
        n(1) = b.numerator * a.denominator;
        n(2) = c.numerator * b.denominator;
        d = a.denominator * b.denominator;
    } else if (b.denominator == c.denominator) {
        n(0) = a.numerator * b.denominator;
        n(1) = b.numerator * a.denominator;
        n(2) = c.numerator * a.denominator;
        d = a.denominator * b.denominator;
    } else {
        n(0) = a.numerator * b.denominator * c.denominator;
        n(1) = b.numerator * a.denominator * c.denominator;
        n(2) = c.numerator * a.denominator * b.denominator;
        d = a.denominator * b.denominator * c.denominator;
    }
}
Point::operator std::string() const {
    return fmt::format("P[({})/{}]", fmt::join(numerator().eval(), " "),
                       denominator());
}
}  // namespace art
