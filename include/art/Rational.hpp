#pragma once
#include <fmt/format.h>

#include <string>

#include "internal/common.hpp"

namespace art {
struct Rational {
    double numerator = 0;
    double denominator = 1;
    Rational() = default;
    Rational(double v) : numerator(v), denominator(1) {}
    Rational(double a, double b) : numerator(a), denominator(b) {}
    Rational(const Rational&) = default;
    Rational(Rational&&) = default;
    Rational& operator=(const Rational&) = default;
    Rational& operator=(Rational&&) = default;
    inline Rational operator+(const Rational& o) const {
        return {numerator * o.denominator + denominator * o.numerator,
                denominator * o.denominator};
    }
    inline Rational operator-(const Rational& o) const {
        return {numerator * o.denominator - denominator * o.numerator,
                denominator * o.denominator};
    }
    inline Rational operator*(const Rational& o) const {
        return {numerator * o.numerator, denominator * o.denominator};
    }
    inline Rational operator/(const Rational& o) const {
        return {numerator * o.denominator, denominator * o.numerator};
    }

    inline Rational operator+(double a) const {
        return {numerator + denominator * a, denominator};
    }
    inline Rational operator-(double a) const {
        return {numerator - denominator * a, denominator};
    }
    inline Rational operator*(double a) const {
        return {numerator * a, denominator};
    }
    inline Rational operator/(double a) const {
        return {numerator, denominator * a};
    }

    inline bool positive() const {
        return (numerator > 0 && denominator > 0) ||
               (numerator < 0 && denominator < 0);
    }

    std::weak_ordering operator<=>(const Rational& o) const;

    inline bool operator==(const Rational& o) const {
        return std::is_eq(*this <=> o);
    }
    inline bool operator<(const Rational& o) const {
        return std::is_lt(*this <=> o);
    }
    inline bool operator>(const Rational& o) const {
        return std::is_gt(*this <=> o);
    }

    inline Rational inverse() const { return {denominator, numerator}; }

    operator double() const { return numerator / denominator; }
    operator std::string() const;
    Rational abs() const {
        return {std::abs(numerator), std::abs(denominator)};
    }
};

std::string format_as(const Rational& a);

Rational min(const Rational& a, const Rational& b);
Rational max(const Rational& a, const Rational& b);
Rational clamp(const Rational& v, const Rational& lower, const Rational& upper);
Rational sqrt(const Rational& r);

}  // namespace art

#if !defined(ART_REDUCE_INLINING)
#include "Rational.hxx"
#endif
