#pragma once
#include <limits>

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
    // Maximum ray parameter — updated during traversal to prune farther hits.
    // Mutable because intersection routines update it while the ray is
    // logically const (standard PBRT pattern).
    mutable Rational tMax = Rational(std::numeric_limits<double>::infinity());

    auto operator()(const Rational &t) const -> Point;
};
ART_API auto format_as(const Ray &r) -> std::string;
} // namespace art
