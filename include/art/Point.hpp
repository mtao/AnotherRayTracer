#if !defined(ART_POINT_HPP)
#define ART_POINT_HPP
#include "art/Rational.hpp"
#include "art/export.hpp"
#include "art/zipper_types.hpp"

namespace art {
class ART_API Point {
   public:
    Point() = default;
    Point(const Vector4d& v);
    Point(double a, double b, double c, double denom = 1.);
    Point(const Rational& a, const Rational& b, const Rational& c);
    template <zipper::concepts::Vector V>
        requires(V::extents_type::static_extent(0) == 3)
    Point(const V& v, double denom = 1.);
    Point(const Point&) = default;
    Point(Point&&) = default;
    Point& operator=(const Point&) = default;
    Point& operator=(Point&&) = default;

    // Element access — returns Rational{m_data(i), m_data(3)}
    Rational operator()(size_t index) const;

    // Structured access — VectorBase views
    auto numerator();
    auto numerator() const;
    double& denominator();
    double denominator() const;

    // Underlying 4-vector (for affine transforms)
    const Vector4d& homogeneous() const;
    Vector4d& homogeneous();

    // Conversion
    operator Vector3d() const;
    operator std::string() const;

    // Static factories
    // Returns a point where all numerator values are the same
    static Point Constant(const Rational& r);
    static Point Constant(double r);
    static Point max_position();
    static Point lowest_position();
    // All coordinates set to +inf
    static Point infinity_position();
    // All coordinates set to -inf
    static Point negative_infinity_position();

    // Member operations
    Rational squaredNorm() const;
    Rational norm() const;
    Point cross(const Point& o) const;

   private:
    Vector4d m_data;
};

// Free-function arithmetic (rational/homogeneous semantics)
ART_API Point operator+(const Point& a, const Point& b);
ART_API Point operator-(const Point& a);
ART_API Point operator-(const Point& a, const Point& b);
ART_API Point operator*(const Point& a, const Rational& r);
ART_API Point operator/(const Point& a, const Rational& r);
ART_API bool operator==(const Point& a, const Point& b);

inline auto format_as(const Point& a) { return std::string(a); }

}  // namespace art

template <>
struct std::formatter<art::Point> : std::formatter<std::string> {
    auto format(const art::Point& p, auto& ctx) const {
        return std::formatter<std::string>::format(std::string(p), ctx);
    }
};

#if !defined(ART_REDUCE_INLINING)
#include "Point.hxx"
#endif
#endif
