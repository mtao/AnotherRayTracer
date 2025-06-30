#pragma once
#include <fmt/format.h>
#include <spdlog/spdlog.h>

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

    inline bool positive() const {
        return (numerator > 0 && denominator > 0) ||
               (numerator < 0 && denominator < 0);
    }

    inline std::partial_ordering operator<=>(const Rational& o) const {
        const bool zero = numerator == 0;
        const bool ozero = o.numerator == 0;
        spdlog::info("{} {}", zero,ozero);
        if (zero && ozero) {
            return std::partial_ordering::equivalent;
        } else if (zero) {
            return o.positive()
                       //
                       ? std::partial_ordering::less
                       : std::partial_ordering::greater;
        } else if (ozero) {
            return positive()
                       //

                       ? std::partial_ordering::less
                       : std::partial_ordering::greater;
            //? std::partial_ordering::greater
            //: std::partial_ordering::less;
        } else {
            spdlog::info("{} {} {} {} {}",numerator,o.denominator,denominator,o.numerator,numerator * o.denominator <=> denominator * o.numerator == 0);
            //spdlog::info("{} {}", *this, o);
            return numerator * o.denominator <=> denominator * o.numerator;
        }
    }

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

inline auto format_as(const Rational& a) {
    return fmt::format("Rational[{}/{}]", a.numerator, a.denominator);
}

Rational min(const Rational& a, const Rational& b);
Rational max(const Rational& a, const Rational& b);
Rational clamp(const Rational& v, const Rational& lower, const Rational& upper);
Rational sqrt(const Rational& r);

}  // namespace art
