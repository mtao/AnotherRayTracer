
#include "art/geometry/Sphere.hpp"

#include <cmath>
#include <numbers>

#include "art/Ray.hpp"

namespace art::geometry {
auto Sphere::bounding_box() const -> Box {
    return Box({Point::Constant(-radius), Point::Constant(radius)});
}
auto Sphere::intersect(const Ray &ray, std::optional<Intersection> &isect) const
    -> bool {
    const auto &d = ray.direction;
    Rational a = ray.direction.norm_powered<2>();
    Rational b(2 * ray.origin.numerator().dot(d), ray.origin.denominator());
    Rational c = ray.origin.squaredNorm() - radius * radius;

    Rational discriminant = b * b - Rational(4) * a * c;
    double disc_double = double(discriminant);
    if (disc_double < 0) { return false; }

    Rational t;
    if (std::abs(disc_double) == 0) {
        t = -b / (2. * a);
    } else {
        Rational sd = sqrt(discriminant);
        Rational t1 = -(b + sd) / (2. * a);
        Rational t2 = -(b - sd) / (2. * a);
        // Pick nearest positive t that is less than tMax
        if (t1 > t2) std::swap(t1, t2);
        if (t1 > Rational(0) && t1 < ray.tMax) {
            t = t1;
        } else if (t2 > Rational(0) && t2 < ray.tMax) {
            t = t2;
        } else {
            return false;
        }
    }

    if (t < Rational(0) || t > ray.tMax) { return false; }
    // Only accept if closer than existing hit
    if (isect && isect->t < t) { return false; }

    ray.tMax = t;

    Point hit_point = ray(t);
    // Divide out to Euclidean for UV and tangent computation
    Vector3d p = Vector3d(hit_point);

    isect.emplace();
    isect->t = t;
    isect->position = hit_point;
    isect->geometric_normal = p; // unnormalized, outward from origin

    // Spherical UV: u = atan2(y, x) / (2pi), v = acos(z/r) / pi
    double r_len = p.norm<2>();
    double phi = std::atan2(p(1), p(0));
    if (phi < 0) phi += 2.0 * std::numbers::pi;
    double theta = std::acos(std::clamp(p(2) / r_len, -1.0, 1.0));

    isect->uv =
        Vector2d{phi / (2.0 * std::numbers::pi), theta / std::numbers::pi};

    // Tangent vectors from spherical parametrization
    // dpdu = d/dphi of (r*sin(theta)*cos(phi), r*sin(theta)*sin(phi),
    //                    r*cos(theta))
    double sin_theta = std::sin(theta);
    isect->dpdu = Vector3d{-p(1), p(0), 0.0}; // (-y, x, 0)
    isect->dpdv =
        Vector3d{p(2) * std::cos(phi),
                 p(2) * std::sin(phi),
                 -r_len * sin_theta}; // (z*cos(phi), z*sin(phi), -r*sin(theta))

    isect->geometry = this;
    isect->primitive_index = 0;

    return true;
}
} // namespace art::geometry
