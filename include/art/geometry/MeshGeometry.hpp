#pragma once
#include <array>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include <quiver/Mesh.hpp>
#include <quiver/attributes/AttributeHandle.hpp>
#include <quiver/spatial/BVH.hpp>

#include "art/geometry/Geometry.hpp"

namespace art::geometry {

/// Triangle mesh geometry backed by quiver's BVH for fast ray intersection.
///
/// Wraps a quiver::Mesh<2> (triangle mesh) with an AABB BVH built over
/// the triangles.  Converts between ART's rational/homogeneous types and
/// quiver's double-precision arrays at the intersection boundary.
class ART_API MeshGeometry : public Geometry {
  public:
    using Vec3 = std::array<double, 3>;
    using Vec2 = std::array<double, 2>;

    /// Construct from raw vertex/triangle data.
    /// Builds quiver mesh, vertex position attribute, skeletons, and BVH.
    MeshGeometry(std::span<const Vec3> vertices,
                 std::span<const std::array<int64_t, 3>> triangles);

    /// Attach per-vertex normals for smooth shading (Phong interpolation).
    auto set_vertex_normals(std::span<const Vec3> normals) -> void;

    /// Attach per-vertex UV coordinates for texture mapping.
    auto set_vertex_uvs(std::span<const Vec2> uvs) -> void;

    auto bounding_box() const -> Box override;
    auto intersect(const Ray &ray, std::optional<Intersection> &isect) const
        -> bool override;

    /// Access underlying mesh data.
    auto mesh() const -> const quiver::Mesh<2> & { return m_mesh; }
    auto triangle_count() const -> size_t;

  private:
    quiver::Mesh<2> m_mesh;
    quiver::attributes::AttributeHandle<Vec3> m_positions;
    quiver::spatial::AABB_BVH<3> m_bvh;

    // Optional per-vertex data (handles into m_mesh's attribute manager)
    quiver::attributes::AttributeHandle<Vec3> m_normals;
    quiver::attributes::AttributeHandle<Vec2> m_uvs;

    /// Compute flat normal for a triangle by index (cross product of edges).
    auto compute_triangle_normal(uint32_t tri_idx) const -> Vector3d;

    /// Compute dpdu/dpdv tangent vectors for a triangle.
    auto compute_tangent_vectors(uint32_t tri_idx) const
        -> std::pair<Vector3d, Vector3d>;

    /// Get the 3 vertex positions of a triangle by index.
    auto get_triangle_vertices(uint32_t tri_idx) const -> std::array<Vec3, 3>;

    /// Build (or rebuild) the AABB BVH over the triangle mesh.
    auto build_bvh() -> void;
};

} // namespace art::geometry
