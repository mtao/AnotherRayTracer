#include "art/Point.hpp"

#include <format>
#include <sstream>

#include <zipper/expression/nullary/Constant.hpp>

namespace art {
Point Point::max_position() {
    return Constant(std::numeric_limits<double>::max());
}
Point Point::lowest_position() {
    return Constant(std::numeric_limits<double>::lowest());
}
Point Point::infinity_position() {
    return Constant(std::numeric_limits<double>::infinity());
}
Point Point::negative_infinity_position() {
    return Constant(-std::numeric_limits<double>::infinity());
}
Point Point::Constant(const Rational& r) {
    Point p;
    p.numerator() = zipper::expression::nullary::Constant(r.numerator);
    p.denominator() = r.denominator;
    return p;
}
Point Point::Constant(double r) { return Constant(Rational(r)); }
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
    auto n = numerator().eval();
    return std::format("P[({} {} {})/{}]", n(0), n(1), n(2),
                       denominator());
}
}  // namespace art
