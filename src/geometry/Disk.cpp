#include "art/geometry/Disk.hpp"

#include <cmath>
#include <numbers>

#include "art/Ray.hpp"

namespace art::geometry {

auto Disk::bounding_box() const -> Box {
    return Box(Point(-1, -1, 0), Point(1, 1, 0));
}

auto Disk::intersect(const Ray &ray, std::optional<Intersection> &isect) const
    -> bool {
    // Disk at z=0, radius 1. First intersect the z=0 plane.
    double dz = ray.direction(2);
    if (std::abs(dz) < 1e-12) {
        return false; // ray parallel to plane
    }

    Rational t = -ray.origin(2) / Rational(dz);

    if (t < Rational(0) || t > ray.tMax) { return false; }
    if (isect && isect->t < t) { return false; }

    Point hit_point = ray(t);
    Vector3d p = Vector3d(hit_point);

    // Check if hit point is within unit disk
    double r2 = p.head<2>().norm_powered<2>();
    if (r2 > 1.0) { return false; }

    ray.tMax = t;

    double r = std::sqrt(r2);
    double phi = std::atan2(p(1), p(0));
    if (phi < 0) phi += 2.0 * std::numbers::pi;

    isect.emplace();
    isect->t = t;
    isect->position = hit_point;
    isect->geometric_normal = Vector3d{0.0, 0.0, 1.0};

    // UV: polar coordinates (r, theta/(2pi))
    isect->uv = Vector2d{r, phi / (2.0 * std::numbers::pi)};

    // Tangent vectors from polar parametrization:
    // p(r, phi) = (r*cos(phi), r*sin(phi), 0)
    // dpdu = dp/dr = (cos(phi), sin(phi), 0)
    // dpdv = dp/dphi = (-r*sin(phi), r*cos(phi), 0)
    double cos_phi = r > 0.0 ? p(0) / r : 1.0;
    double sin_phi = r > 0.0 ? p(1) / r : 0.0;
    isect->dpdu = Vector3d{cos_phi, sin_phi, 0.0};
    isect->dpdv = Vector3d{-r * sin_phi, r * cos_phi, 0.0};

    isect->geometry = this;
    isect->primitive_index = 0;

    return true;
}

} // namespace art::geometry
