#pragma once
#include "art/zipper_types.hpp"
#include "art/rational.hpp"

namespace art {
struct Point : public Vector4d {
    using Base = Vector4d;
    Point() = default;
    Point(const Base& v) : Base(v) {}
    static Point Constant(const Rational& r);
    //Point(const Vector3d& v) : Base(v.homogeneous()) {}
    Point(const Vector3d& v, double denom = 1.) : Base(v.homogeneous()) {
        denominator() = denom;
    }
    Point(double a, double b, double c, double denom = 1.) {
        Base::operator()(0) = a;
        Base::operator()(1) = b;
        Base::operator()(2) = c;
        Base::operator()(3) = denom;
    }
    Point(const Rational& a, const Rational& b, const Rational& c);
    Point(const Point&) = default;
    Point(Point&&) = default;

    Rational operator()(size_t index) const {
        return {Base::operator()(index), Base::operator()(3)};
    }

    auto numerator() { return Base::head<3>(); }
    auto numerator() const { return Base::head<3>(); }
    double& denominator() { return Base::operator()(3); }
    double denominator() const { return Base::operator()(3); }

    const Vector4d& homogeneous() const { return *this; }
    Vector4d& homogeneous() { return *this; }

    Point& operator=(const Point&) = default;
    Point& operator=(Point&&) = default;
    inline Point operator+(const Point& o) const {
        return {numerator() * o.denominator() + denominator() * o.numerator(),
                denominator() * o.denominator()};
    }
    inline Point operator-() const {
        Point r = *this;
        r.denominator() = -denominator();
        return r;
    }
    inline Point operator-(const Point& o) const {
        return {numerator() * o.denominator() - denominator() * o.numerator(),
                denominator() * o.denominator()};
    }
    inline Point operator*(const Rational& o) const {
        return {numerator() * o.numerator, denominator() * o.denominator};
    }

    inline Point operator/(const Rational& o) const {
        return {numerator() * o.denominator, denominator() * o.numerator};
    }
    inline bool operator==(const Point& o) const {
        return numerator() * o.denominator() == denominator() * o.numerator();
    }

    inline Point cross(const Point& o) const {
        return {numerator().cross(o.numerator()),
                denominator() * o.denominator()};
    }

    Rational squaredNorm() const {
        return {numerator().squaredNorm(), denominator() * denominator()};
    }

    Rational norm() const { return {numerator().norm(), denominator()}; }

    operator Vector3d() const { return numerator() / denominator(); }
    operator std::string() const;
};
}  // namespace art
