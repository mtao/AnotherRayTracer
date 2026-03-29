#include "art/geometry/mesh_utils.hpp"

#include <cmath>

namespace art::geometry {

auto make_mesh_geometry(std::span<const std::array<double, 3>> vertices,
                        std::span<const std::array<int64_t, 3>> triangles)
    -> std::shared_ptr<MeshGeometry> {
    return std::make_shared<MeshGeometry>(vertices, triangles);
}

auto make_cube_mesh() -> std::shared_ptr<MeshGeometry> {
    // Unit cube from (-0.5,-0.5,-0.5) to (0.5,0.5,0.5)
    // 8 vertices, 12 triangles (2 per face)
    std::vector<std::array<double, 3>> verts = {
        {-0.5, -0.5, -0.5}, // 0
        {0.5, -0.5, -0.5}, // 1
        {0.5, 0.5, -0.5}, // 2
        {-0.5, 0.5, -0.5}, // 3
        {-0.5, -0.5, 0.5}, // 4
        {0.5, -0.5, 0.5}, // 5
        {0.5, 0.5, 0.5}, // 6
        {-0.5, 0.5, 0.5}, // 7
    };

    std::vector<std::array<int64_t, 3>> tris = {
        // -z face (0,1,2,3)
        {0, 2, 1},
        {0, 3, 2},
        // +z face (4,5,6,7)
        {4, 5, 6},
        {4, 6, 7},
        // -y face (0,1,5,4)
        {0, 1, 5},
        {0, 5, 4},
        // +y face (3,2,6,7)
        {3, 7, 6},
        {3, 6, 2},
        // -x face (0,3,7,4)
        {0, 4, 7},
        {0, 7, 3},
        // +x face (1,2,6,5)
        {1, 2, 6},
        {1, 6, 5},
    };

    return make_mesh_geometry(verts, tris);
}

auto make_tetrahedron_mesh() -> std::shared_ptr<MeshGeometry> {
    // Regular tetrahedron centered at origin
    double a = 1.0 / std::sqrt(3.0);
    std::vector<std::array<double, 3>> verts = {
        {a, a, a}, // 0
        {a, -a, -a}, // 1
        {-a, a, -a}, // 2
        {-a, -a, a}, // 3
    };

    std::vector<std::array<int64_t, 3>> tris = {
        {0, 1, 2},
        {0, 3, 1},
        {0, 2, 3},
        {1, 3, 2},
    };

    return make_mesh_geometry(verts, tris);
}

auto make_single_triangle_mesh(const std::array<double, 3> &v0,
                               const std::array<double, 3> &v1,
                               const std::array<double, 3> &v2)
    -> std::shared_ptr<MeshGeometry> {
    std::vector<std::array<double, 3>> verts = {v0, v1, v2};
    std::vector<std::array<int64_t, 3>> tris = {{0, 1, 2}};
    return make_mesh_geometry(verts, tris);
}

} // namespace art::geometry
