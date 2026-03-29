#include "art/geometry/Cylinder.hpp"

#include <cmath>
#include <numbers>

#include "art/Ray.hpp"

namespace art::geometry {

auto Cylinder::bounding_box() const -> Box {
    return Box(Point(-1, -1, 0), Point(1, 1, 1));
}

auto Cylinder::intersect(const Ray &ray,
                         std::optional<Intersection> &isect) const -> bool {
    // Unit cylinder: x^2 + y^2 = 1, z in [0, 1]
    // Substituting ray: (ox + t*dx)^2 + (oy + t*dy)^2 = 1
    // => (dx^2 + dy^2)*t^2 + 2*(ox*dx + oy*dy)*t + (ox^2 + oy^2 - 1) = 0

    Vector3d o = Vector3d(Point(ray.origin));
    const auto &d = ray.direction;

    auto d_xy = d.head<2>();
    auto o_xy = o.head<2>();

    double a = d_xy.norm_powered<2>();
    if (std::abs(a) < 1e-12) {
        return false; // ray parallel to z-axis
    }

    double b = 2.0 * o_xy.dot(d_xy);
    double c = o_xy.norm_powered<2>() - 1.0;

    double disc = b * b - 4.0 * a * c;
    if (disc < 0) { return false; }

    double sqrt_disc = std::sqrt(disc);
    double inv_2a = 1.0 / (2.0 * a);
    double t1 = (-b - sqrt_disc) * inv_2a;
    double t2 = (-b + sqrt_disc) * inv_2a;

    // Try nearest positive t within z range [0, 1]
    auto try_hit = [&](double t_val) -> bool {
        if (t_val <= 0.0) return false;

        Rational t_rat(t_val);
        if (t_rat > ray.tMax) return false;
        if (isect && isect->t < t_rat) return false;

        // Check z range
        double z = o(2) + t_val * d(2);
        if (z < 0.0 || z > 1.0) return false;

        double x = o(0) + t_val * d(0);
        double y = o(1) + t_val * d(1);

        ray.tMax = t_rat;

        double phi = std::atan2(y, x);
        if (phi < 0) phi += 2.0 * std::numbers::pi;

        isect.emplace();
        isect->t = t_rat;
        isect->position = Point(x, y, z);
        // Radial normal (outward from z-axis)
        isect->geometric_normal = Vector3d{x, y, 0.0};
        // UV: (theta/(2pi), z)
        isect->uv = Vector2d{phi / (2.0 * std::numbers::pi), z};
        // dpdu: tangent along circumference
        isect->dpdu = Vector3d{-y, x, 0.0};
        // dpdv: tangent along z-axis
        isect->dpdv = Vector3d{0.0, 0.0, 1.0};
        isect->geometry = this;
        isect->primitive_index = 0;

        return true;
    };

    if (try_hit(t1)) return true;
    if (try_hit(t2)) return true;
    return false;
}

} // namespace art::geometry
