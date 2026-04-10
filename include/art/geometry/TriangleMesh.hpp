#pragma once

#include <array>
#include <memory>
#include <span>
#include <vector>

#include "art/geometry/Geometry.hpp"
#include "art/zipper_types.hpp"

namespace art::geometry {

/// Triangle mesh geometry for ray tracing.
///
/// Stores vertex positions and triangle face indices. Computes per-face
/// normals on construction. Uses Moller-Trumbore intersection.
class ART_API TriangleMesh : public Geometry {
  public:
    using Face = std::array<size_t, 3>;

    /// Construct from vertex positions and triangle indices.
    TriangleMesh(std::vector<Vector3d> vertices, std::vector<Face> faces);

    auto bounding_box() const -> Box override;

    auto intersect(const Ray &ray, std::optional<Intersection> &isect) const
        -> bool override;

    auto vertices() const -> std::span<const Vector3d>;
    auto faces() const -> std::span<const Face>;
    auto face_normals() const -> std::span<const Vector3d>;

  private:
    std::vector<Vector3d> m_vertices;
    std::vector<Face> m_faces;
    std::vector<Vector3d> m_face_normals;
    Box m_bbox;

    void compute_normals();
    void compute_bbox();
};

} // namespace art::geometry
