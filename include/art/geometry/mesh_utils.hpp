#pragma once
#include <array>
#include <memory>
#include <span>
#include <vector>

#include "art/export.hpp"
#include "art/geometry/MeshGeometry.hpp"

namespace art::geometry {

/// Create a MeshGeometry from raw vertex/index data.
ART_API auto
    make_mesh_geometry(std::span<const std::array<double, 3>> vertices,
                       std::span<const std::array<int64_t, 3>> triangles)
        -> std::shared_ptr<MeshGeometry>;

/// Create a unit cube mesh (axis-aligned, from (-0.5,-0.5,-0.5) to
/// (0.5,0.5,0.5)). Each face is split into 2 triangles (12 triangles total, 8
/// vertices).
ART_API auto make_cube_mesh() -> std::shared_ptr<MeshGeometry>;

/// Create a regular tetrahedron mesh.
ART_API auto make_tetrahedron_mesh() -> std::shared_ptr<MeshGeometry>;

/// Create a single-triangle mesh for testing.
ART_API auto make_single_triangle_mesh(const std::array<double, 3> &v0,
                                       const std::array<double, 3> &v1,
                                       const std::array<double, 3> &v2)
    -> std::shared_ptr<MeshGeometry>;

} // namespace art::geometry
