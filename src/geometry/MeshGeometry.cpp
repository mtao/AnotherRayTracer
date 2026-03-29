#include "art/geometry/MeshGeometry.hpp"

#include <quiver/attributes/IncidentFaceIndices.hpp>
#include <quiver/spatial/bounding_volume.hpp>
#include <quiver/spatial/queries.hpp>

#include "art/Ray.hpp"

namespace art::geometry {

MeshGeometry::MeshGeometry(std::span<const Vec3> vertices,
                           std::span<const std::array<int64_t, 3>> triangles) {
    // Build quiver mesh from triangle indices
    m_mesh = quiver::Mesh<2>::from_vertex_indices(triangles);
    m_mesh.build_skeleton<0>();
    m_mesh.build_skeleton<1>();

    // Create and populate vertex position attribute
    m_positions = m_mesh.create_attribute<Vec3>("vertex_positions", 0);
    for (size_t i = 0; i < vertices.size(); ++i) {
        m_positions[i] = vertices[i];
    }

    build_bvh();
}

auto MeshGeometry::set_vertex_normals(std::span<const Vec3> normals) -> void {
    m_normals = m_mesh.create_attribute<Vec3>("vertex_normals", 0);
    for (size_t i = 0; i < normals.size(); ++i) { m_normals[i] = normals[i]; }
}

auto MeshGeometry::set_vertex_uvs(std::span<const Vec2> uvs) -> void {
    m_uvs = m_mesh.create_attribute<Vec2>("vertex_uvs", 0);
    for (size_t i = 0; i < uvs.size(); ++i) { m_uvs[i] = uvs[i]; }
}

auto MeshGeometry::bounding_box() const -> Box {
    const auto &bounds = m_bvh.root_bounds();
    // KDOP<3,3> stores per-axis min/max via .min(axis) / .max(axis)
    return Box(Point(bounds.min(0), bounds.min(1), bounds.min(2)),
               Point(bounds.max(0), bounds.max(1), bounds.max(2)));
}

auto MeshGeometry::intersect(const Ray &ray,
                             std::optional<Intersection> &isect) const -> bool {
    // Convert art::Ray to quiver-compatible arrays
    // Origin: divide out homogeneous coordinate (one division)
    Vector3d origin_vec = Vector3d(Point(ray.origin));
    Vec3 origin_arr = {origin_vec(0), origin_vec(1), origin_vec(2)};
    Vec3 dir_arr = {ray.direction(0), ray.direction(1), ray.direction(2)};

    // Cast ray via quiver's BVH-accelerated intersection
    quiver::attributes::AttributeHandle<const Vec3> cpos(m_positions);
    auto result =
        quiver::spatial::cast_ray<2, 3>(m_mesh,
                                        cpos,
                                        m_bvh,
                                        std::span<const double, 3>(origin_arr),
                                        std::span<const double, 3>(dir_arr));

    if (!result) { return false; }

    // Check tMax
    Rational t_hit(result->t);
    if (t_hit > ray.tMax) { return false; }
    if (isect && isect->t < t_hit) { return false; }

    ray.tMax = t_hit;

    // Build intersection record
    isect.emplace();
    isect->t = t_hit;
    isect->position =
        Point(result->point[0], result->point[1], result->point[2]);
    isect->geometric_normal = compute_triangle_normal(result->simplex_index);
    isect->geometry = this;
    isect->primitive_index = result->simplex_index;

    // UV coordinates
    const auto &bary = result->barycentric;
    if (m_uvs.valid()) {
        // Interpolate per-vertex UVs using barycentrics
        quiver::attributes::IncidentFaceIndices<2, 0, 2> face_idx(
            m_mesh.skeleton<0>());
        auto vi = face_idx.get_indices(result->simplex_index);

        const auto &uv0 = m_uvs[vi[0]];
        const auto &uv1 = m_uvs[vi[1]];
        const auto &uv2 = m_uvs[vi[2]];
        isect->uv =
            Vector2d{bary[0] * uv0[0] + bary[1] * uv1[0] + bary[2] * uv2[0],
                     bary[0] * uv0[1] + bary[1] * uv1[1] + bary[2] * uv2[1]};
    } else {
        // Use barycentrics as UV
        isect->uv = Vector2d{bary[0], bary[1]};
    }

    auto [dpdu, dpdv] = compute_tangent_vectors(result->simplex_index);
    isect->dpdu = dpdu;
    isect->dpdv = dpdv;

    return true;
}

auto MeshGeometry::triangle_count() const -> size_t {
    return m_mesh.simplex_count(2);
}

auto MeshGeometry::get_triangle_vertices(uint32_t tri_idx) const
    -> std::array<Vec3, 3> {
    quiver::attributes::IncidentFaceIndices<2, 0, 2> face_idx(
        m_mesh.skeleton<0>());
    auto vi = face_idx.get_indices(tri_idx);

    quiver::attributes::AttributeHandle<const Vec3> cpos(m_positions);
    return {cpos[vi[0]], cpos[vi[1]], cpos[vi[2]]};
}

auto MeshGeometry::compute_triangle_normal(uint32_t tri_idx) const -> Vector3d {
    auto verts = get_triangle_vertices(tri_idx);
    Vector3d v0{verts[0][0], verts[0][1], verts[0][2]};
    Vector3d v1{verts[1][0], verts[1][1], verts[1][2]};
    Vector3d v2{verts[2][0], verts[2][1], verts[2][2]};

    Vector3d e1 = (v1 - v0).eval();
    Vector3d e2 = (v2 - v0).eval();
    return e1.cross(e2);
}

auto MeshGeometry::compute_tangent_vectors(uint32_t tri_idx) const
    -> std::pair<Vector3d, Vector3d> {
    auto verts = get_triangle_vertices(tri_idx);
    Vector3d v0{verts[0][0], verts[0][1], verts[0][2]};
    Vector3d v1{verts[1][0], verts[1][1], verts[1][2]};
    Vector3d v2{verts[2][0], verts[2][1], verts[2][2]};

    Vector3d e1 = (v1 - v0).eval();
    Vector3d e2 = (v2 - v0).eval();

    // If UVs are provided, compute dpdu/dpdv from UV differentials (PBRT)
    if (m_uvs.valid()) {
        quiver::attributes::IncidentFaceIndices<2, 0, 2> face_idx(
            m_mesh.skeleton<0>());
        auto vi = face_idx.get_indices(tri_idx);

        const auto &uv0 = m_uvs[vi[0]];
        const auto &uv1 = m_uvs[vi[1]];
        const auto &uv2 = m_uvs[vi[2]];

        double du1 = uv1[0] - uv0[0];
        double dv1 = uv1[1] - uv0[1];
        double du2 = uv2[0] - uv0[0];
        double dv2 = uv2[1] - uv0[1];

        double det = du1 * dv2 - dv1 * du2;
        if (std::abs(det) > 1e-12) {
            double inv_det = 1.0 / det;
            Vector3d dpdu = ((dv2 * e1 - dv1 * e2) * inv_det).eval();
            Vector3d dpdv = ((-du2 * e1 + du1 * e2) * inv_det).eval();
            return {dpdu, dpdv};
        }
    }

    // Fallback: use edge vectors directly
    return {e1, e2};
}

auto MeshGeometry::build_bvh() -> void {
    quiver::attributes::AttributeHandle<const Vec3> cpos(m_positions);
    m_bvh = quiver::spatial::make_bvh<2, 3>(m_mesh, cpos);
}

} // namespace art::geometry
