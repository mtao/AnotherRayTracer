#pragma once
#include "art/zipper_types.hpp"
#include "art/point.hpp"
#include "art/rational.hpp"

namespace art {
struct Line : public Vector6d {
    using Base = Vector6d;
    Line() = default;
    Line(const Base& v) : Base(v) {}
    auto moment() { return Base::head<3>(); }
    auto direction() { return Base::tail<3>(); }
    auto moment() const { return Base::head<3>(); }
    auto direction() const { return Base::tail<3>(); }
    Line(const Vector3d& a, const Vector3d& b) {
        moment() = a.cross(b);
        direction() = b - a;
    }
    Line(const Line&) = default;
    Line(Line&&) = default;
    Line& operator=(const Line&) = default;
    Line& operator=(Line&&) = default;


    Point momentAsPoint() const { return Vector3d{Base::head<3>()}; }
    Point directionAsPoint() const { return Vector3d{Base::tail<3>()}; }

    Rational distance(const Point& p) const;
    Rational distance(const Line& l) const;
    double reciprocalProduct(const Line& l) const;

    Point smallestPoint() const;
    /*

    inline Line operator+(const Line& o) const {
        return {numerator() * o.denominator() + denominator() * o.numerator(),
                denominator() * o.denominator()};
    }
    inline Line operator-() const {
        Line r = *this;
        r.denominator() = -denominator();
        return r;
    }
    inline Line operator-(const Line& o) const {
        return {numerator() * o.denominator() - denominator() * o.numerator(),
                denominator() * o.denominator()};
    }
    inline Line operator*(const Rational& o) const {
        return {numerator() * o.numerator, denominator() * o.denominator};
    }

    inline bool operator==(const Line& o) const {
        return numerator() * o.denominator() == denominator() * o.numerator();
    }

    Rational squaredNorm() const {
        return {numerator().squaredNorm(), denominator() * denominator()};
    }

    operator Vector3d() const { return numerator() / denominator(); }
    */
    operator std::string() const;
};
}  // namespace art
