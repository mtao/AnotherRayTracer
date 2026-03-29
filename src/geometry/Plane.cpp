#include "art/geometry/Plane.hpp"

#include <limits>

#include "art/Ray.hpp"

namespace art::geometry {

auto Plane::bounding_box() const -> Box {
    // Infinite plane — use very large bounding box
    constexpr double big = 1e6;
    return Box(Point(-big, -big, 0), Point(big, big, 0));
}

auto Plane::intersect(const Ray &ray, std::optional<Intersection> &isect) const
    -> bool {
    // Plane at z=0: ray.origin.z + t * ray.direction.z = 0
    double dz = ray.direction(2);
    if (std::abs(dz) < 1e-12) {
        return false; // ray parallel to plane
    }

    // t = -origin.z / direction.z
    Rational t = -ray.origin(2) / Rational(dz);

    if (t < Rational(0) || t > ray.tMax) { return false; }
    if (isect && isect->t < t) { return false; }

    ray.tMax = t;

    Point hit_point = ray(t);
    Vector3d p = Vector3d(hit_point);

    isect.emplace();
    isect->t = t;
    isect->position = hit_point;
    isect->geometric_normal = Vector3d{0.0, 0.0, 1.0};
    isect->uv = Vector2d{p(0), p(1)};
    isect->dpdu = Vector3d{1.0, 0.0, 0.0};
    isect->dpdv = Vector3d{0.0, 1.0, 0.0};
    isect->geometry = this;
    isect->primitive_index = 0;

    return true;
}

} // namespace art::geometry
