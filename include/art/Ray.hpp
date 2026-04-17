#if !defined(ART_RAY_HPP)
#define ART_RAY_HPP
#include "art/Intersection.hpp"
#include "art/Point.hpp"
#include "art/export.hpp"
#include "art/geometry/Box.hpp"

namespace art {
namespace geometry {
class Box;
}
struct ART_API Ray {
    Point origin;
    Vector3d direction;

    Point operator()(const Rational& t) const;
};
ART_API std::string format_as(const Ray& r);
}  // namespace art
#endif
