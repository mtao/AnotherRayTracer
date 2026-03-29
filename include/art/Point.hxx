#if !defined(ART_POINT_HXX)
#define ART_POINT_HXX
#include "art/Point.hpp"

#include <limits>

#include <zipper/expression/nullary/Constant.hpp>

namespace art {

// --- Structured access (defined first — deduced return types must be
//     visible before any code that calls them) ---

ART_INLINE auto Point::numerator() { return m_data.head<3>(); }
ART_INLINE auto Point::numerator() const { return m_data.head<3>(); }
ART_INLINE double& Point::denominator() { return m_data(3); }
ART_INLINE double Point::denominator() const { return m_data(3); }

// --- Element access ---

ART_INLINE Rational Point::operator()(size_t index) const {
    return {m_data(index), m_data(3)};
}

// --- Constructors ---

ART_INLINE Point::Point(const Vector4d& v) : m_data(v) {}

ART_INLINE Point::Point(double a, double b, double c, double denom) {
    m_data(0) = a;
    m_data(1) = b;
    m_data(2) = c;
    m_data(3) = denom;
}

ART_INLINE Point::Point(const Rational& a, const Rational& b,
                         const Rational& c) {
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

template <zipper::concepts::Vector V>
    requires(V::extents_type::static_extent(0) == 3)
ART_INLINE Point::Point(const V& v, double denom) {
    numerator() = v;
    denominator() = denom;
}

// --- Underlying 4-vector ---

ART_INLINE const Vector4d& Point::homogeneous() const { return m_data; }
ART_INLINE Vector4d& Point::homogeneous() { return m_data; }

// --- Conversion ---

ART_INLINE Point::operator Vector3d() const {
    return numerator() / denominator();
}

// --- Static factories ---

ART_INLINE Point Point::Constant(const Rational& r) {
    Point p;
    p.numerator() =
        zipper::expression::nullary::Constant<double, 3>(r.numerator);
    p.denominator() = r.denominator;
    return p;
}

ART_INLINE Point Point::Constant(double r) { return Constant(Rational(r)); }

ART_INLINE Point Point::max_position() {
    return Constant(std::numeric_limits<double>::max());
}

ART_INLINE Point Point::lowest_position() {
    return Constant(std::numeric_limits<double>::lowest());
}

ART_INLINE Point Point::infinity_position() {
    return Constant(std::numeric_limits<double>::infinity());
}

ART_INLINE Point Point::negative_infinity_position() {
    return Constant(-std::numeric_limits<double>::infinity());
}

// --- Member operations ---

ART_INLINE Rational Point::squaredNorm() const {
    return {numerator().norm_powered<2>(), denominator() * denominator()};
}

ART_INLINE Rational Point::norm() const {
    return {numerator().norm(), denominator()};
}

ART_INLINE Point Point::cross(const Point& o) const {
    return Point(numerator().cross(o.numerator()),
                 denominator() * o.denominator());
}

// --- Free-function arithmetic ---

ART_INLINE Point operator+(const Point& a, const Point& b) {
    return Point(a.numerator() * b.denominator() +
                     a.denominator() * b.numerator(),
                 a.denominator() * b.denominator());
}

ART_INLINE Point operator-(const Point& a) {
    Point r = a;
    r.denominator() = -a.denominator();
    return r;
}

ART_INLINE Point operator-(const Point& a, const Point& b) {
    return Point(a.numerator() * b.denominator() -
                     a.denominator() * b.numerator(),
                 a.denominator() * b.denominator());
}

ART_INLINE Point operator*(const Point& a, const Rational& r) {
    return Point(a.numerator() * r.numerator,
                 a.denominator() * r.denominator);
}

ART_INLINE Point operator/(const Point& a, const Rational& r) {
    return Point(a.numerator() * r.denominator,
                 a.denominator() * r.numerator);
}

ART_INLINE bool operator==(const Point& a, const Point& b) {
    return a.numerator() * b.denominator() == a.denominator() * b.numerator();
}

}  // namespace art
#endif
