#pragma once
#include "art/Intersection.hpp"
#include "art/Point.hpp"
#include "art/geometry/Box.hpp"

namespace art {
namespace geometry {
class Box;
}
struct Ray {
    Point origin;
    Vector3d direction;

    Point operator()(const Rational& t) const;
};
std::string format_as(const Ray& r);
}  // namespace art
