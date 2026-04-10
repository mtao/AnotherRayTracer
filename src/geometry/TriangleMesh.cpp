#include "art/geometry/TriangleMesh.hpp"

#include <cmath>
#include <limits>

#include "art/Point.hpp"
#include "art/Ray.hpp"

namespace art::geometry {

TriangleMesh::TriangleMesh(std::vector<Vector3d> vertices,
                           std::vector<Face> faces)
  : m_vertices(std::move(vertices)), m_faces(std::move(faces)) {
    compute_normals();
    compute_bbox();
}

auto TriangleMesh::bounding_box() const -> Box { return m_bbox; }

auto TriangleMesh::intersect(const Ray &ray,
                             std::optional<Intersection> &isect) const -> bool {
    // Moller-Trumbore intersection for each triangle.
    // Linear scan — acceptable for moderate meshes; BVH is a future addition.
    constexpr double eps = 1e-12;

    // Pre-extract the Euclidean ray origin (one division up front).
    Vector3d origin = ray.origin;
    const auto &dir = ray.direction;

    bool any_hit = false;

    for (size_t fi = 0; fi < m_faces.size(); ++fi) {
        const auto &[i0, i1, i2] = m_faces[fi];
        const auto &v0 = m_vertices[i0];
        const auto &v1 = m_vertices[i1];
        const auto &v2 = m_vertices[i2];

        Vector3d e1 = v1 - v0;
        Vector3d e2 = v2 - v0;
        Vector3d h = dir.cross(e2);
        double a = e1.dot(h);

        if (std::abs(a) < eps) {
            continue; // Ray parallel to triangle.
        }

        double f = 1.0 / a;
        Vector3d s = origin - v0;
        double u = f * s.dot(h);
        if (u < 0.0 || u > 1.0) { continue; }

        Vector3d q = s.cross(e1);
        double v = f * dir.dot(q);
        if (v < 0.0 || u + v > 1.0) { continue; }

        double t = f * e2.dot(q);
        if (t < eps) {
            continue; // Behind the ray origin.
        }

        Rational t_rat(t);
        if (!isect || isect->t > t_rat) {
            isect.emplace();
            isect->t = t_rat;
            isect->position = ray(t_rat);
            isect->normal = m_face_normals[fi];
            any_hit = true;
        }
    }

    return any_hit;
}

auto TriangleMesh::vertices() const -> std::span<const Vector3d> {
    return m_vertices;
}

auto TriangleMesh::faces() const -> std::span<const Face> { return m_faces; }

auto TriangleMesh::face_normals() const -> std::span<const Vector3d> {
    return m_face_normals;
}

void TriangleMesh::compute_normals() {
    m_face_normals.resize(m_faces.size());
    for (size_t fi = 0; fi < m_faces.size(); ++fi) {
        const auto &[i0, i1, i2] = m_faces[fi];
        Vector3d e1 = m_vertices[i1] - m_vertices[i0];
        Vector3d e2 = m_vertices[i2] - m_vertices[i0];
        Vector3d n = e1.cross(e2);
        double len = n.norm<2>();
        if (len > 1e-15) {
            m_face_normals[fi] = n / len;
        } else {
            m_face_normals[fi] = Vector3d({0.0, 0.0, 1.0});
        }
    }
}

void TriangleMesh::compute_bbox() {
    if (m_vertices.empty()) {
        m_bbox = Box();
        return;
    }

    m_bbox = Box({Point(m_vertices[0]), Point(m_vertices[0])});
    for (size_t i = 1; i < m_vertices.size(); ++i) {
        m_bbox.expand(Point(m_vertices[i]));
    }
}

} // namespace art::geometry
