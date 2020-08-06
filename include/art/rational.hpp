#pragma once
#include <string>

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

    inline bool operator==(const Rational& o) const {
        return numerator * o.denominator == denominator * o.numerator;
    }
    inline bool operator<(const Rational& o) const {
        return numerator * o.denominator < denominator * o.numerator;
    }

    inline Rational inverse() const { return {denominator, numerator}; }

    operator double() const { return numerator / denominator; }
    operator std::string() const;
    Rational abs() const {
        return { std::abs(numerator), std::abs(denominator) };
    }
};

Rational min(const Rational& a, const Rational& b);
Rational max(const Rational& a, const Rational& b);
Rational clamp(const Rational& v, const Rational& lower, const Rational& upper);
Rational sqrt(const Rational& r);

}  // namespace art
