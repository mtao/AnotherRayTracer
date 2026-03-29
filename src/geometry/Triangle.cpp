#include "art/geometry/Triangle.hpp"

#include <cmath>

#include "art/Ray.hpp"

namespace art::geometry {

Triangle::Triangle(Vector3d v0, Vector3d v1, Vector3d v2)
  : m_vertices{v0, v1, v2} {}

Triangle::Triangle(Vector3d v0,
                   Vector3d v1,
                   Vector3d v2,
                   Vector3d n0,
                   Vector3d n1,
                   Vector3d n2)
  : m_vertices{v0, v1, v2}, m_normals{std::array<Vector3d, 3>{n0, n1, n2}} {}

Triangle::Triangle(Vector3d v0,
                   Vector3d v1,
                   Vector3d v2,
                   Vector2d uv0,
                   Vector2d uv1,
                   Vector2d uv2)
  : m_vertices{v0, v1, v2}, m_uvs{std::array<Vector2d, 3>{uv0, uv1, uv2}} {}

auto Triangle::bounding_box() const -> Box {
    const auto &v0 = m_vertices[0];
    const auto &v1 = m_vertices[1];
    const auto &v2 = m_vertices[2];

    Vector3d lo{std::min({v0(0), v1(0), v2(0)}),
                std::min({v0(1), v1(1), v2(1)}),
                std::min({v0(2), v1(2), v2(2)})};
    Vector3d hi{std::max({v0(0), v1(0), v2(0)}),
                std::max({v0(1), v1(1), v2(1)}),
                std::max({v0(2), v1(2), v2(2)})};

    return Box(Point(lo(0), lo(1), lo(2)), Point(hi(0), hi(1), hi(2)));
}

auto Triangle::intersect(const Ray &ray,
                         std::optional<Intersection> &isect) const -> bool {
    // Moller-Trumbore ray-triangle intersection
    const auto &v0 = m_vertices[0];
    const auto &v1 = m_vertices[1];
    const auto &v2 = m_vertices[2];

    Vector3d edge1 = (v1 - v0).eval();
    Vector3d edge2 = (v2 - v0).eval();

    Vector3d o = Vector3d(Point(ray.origin));
    const auto &d = ray.direction;

    Vector3d h = d.cross(edge2);
    double a = edge1.dot(h);

    if (std::abs(a) < 1e-12) {
        return false; // ray parallel to triangle
    }

    double f = 1.0 / a;
    Vector3d s = (o - v0).eval();
    double u = f * s.dot(h);
    if (u < 0.0 || u > 1.0) { return false; }

    Vector3d q = s.cross(edge1);
    double v = f * d.dot(q);
    if (v < 0.0 || u + v > 1.0) { return false; }

    double t_val = f * edge2.dot(q);
    if (t_val <= 0.0) { return false; }

    Rational t(t_val);
    if (t > ray.tMax) { return false; }
    if (isect && isect->t < t) { return false; }

    ray.tMax = t;

    double w = 1.0 - u - v; // barycentric w for v0

    isect.emplace();
    isect->t = t;

    // Interpolate position from barycentrics for better precision
    Vector3d pos = (w * v0 + u * v1 + v * v2).eval();
    isect->position = Point(pos(0), pos(1), pos(2));

    // Geometric normal: cross product of edges (not normalized)
    isect->geometric_normal = edge1.cross(edge2);

    // UV coordinates
    if (m_uvs) {
        const auto &uv0 = (*m_uvs)[0];
        const auto &uv1 = (*m_uvs)[1];
        const auto &uv2 = (*m_uvs)[2];
        isect->uv = (w * uv0 + u * uv1 + v * uv2).eval();
    } else {
        // Use barycentrics as UV
        isect->uv = Vector2d{u, v};
    }

    // Compute dpdu/dpdv from edge vectors and UV differentials
    // Following PBRT: solve [duv1; duv2] * [dpdu; dpdv]^T = [e1; e2]^T
    if (m_uvs) {
        const auto &uv0 = (*m_uvs)[0];
        const auto &uv1 = (*m_uvs)[1];
        const auto &uv2 = (*m_uvs)[2];

        double du1 = uv1(0) - uv0(0);
        double dv1 = uv1(1) - uv0(1);
        double du2 = uv2(0) - uv0(0);
        double dv2 = uv2(1) - uv0(1);

        double det = du1 * dv2 - dv1 * du2;
        if (std::abs(det) > 1e-12) {
            double inv_det = 1.0 / det;
            isect->dpdu = (dv2 * edge1 - dv1 * edge2).eval() * inv_det;
            isect->dpdv = (-du2 * edge1 + du1 * edge2).eval() * inv_det;
        } else {
            // Degenerate UV: use edge vectors directly
            isect->dpdu = edge1;
            isect->dpdv = edge2;
        }
    } else {
        // No UVs: dpdu/dpdv correspond to barycentric u,v directions
        isect->dpdu = edge1;
        isect->dpdv = edge2;
    }

    isect->geometry = this;
    isect->primitive_index = 0;

    return true;
}

} // namespace art::geometry
