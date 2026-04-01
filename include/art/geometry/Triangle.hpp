#pragma once
#include <array>
#include <optional>

#include "art/export.hpp"
#include "art/geometry/Geometry.hpp"

namespace art::geometry {
// Single triangle with 3 vertices, optional per-vertex normals and UVs.
// This is NOT a mesh — just one triangle for scene description.
// For triangle meshes, use MeshGeometry with quiver.
class ART_API Triangle : public Geometry {
  public:
    Triangle(Vector3d v0, Vector3d v1, Vector3d v2);
    Triangle(Vector3d v0,
             Vector3d v1,
             Vector3d v2,
             Vector3d n0,
             Vector3d n1,
             Vector3d n2);
    Triangle(Vector3d v0,
             Vector3d v1,
             Vector3d v2,
             Vector2d uv0,
             Vector2d uv1,
             Vector2d uv2);

    auto bounding_box() const -> Box override;
    auto intersect(const Ray &ray, std::optional<Intersection> &isect) const
        -> bool override;

    // Accessors for serialization / introspection
    auto vertices() const -> const std::array<Vector3d, 3> &;
    auto normals() const -> const std::optional<std::array<Vector3d, 3>> &;
    auto uvs() const -> const std::optional<std::array<Vector2d, 3>> &;

  private:
    std::array<Vector3d, 3> m_vertices;
    std::optional<std::array<Vector3d, 3>> m_normals;
    std::optional<std::array<Vector2d, 3>> m_uvs;
};
} // namespace art::geometry
