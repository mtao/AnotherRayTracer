/// @file src/lua/bind_geometry.cpp
/// Lua bindings for ART geometry types.

#include "bind_internal.hpp"

#include "art/geometry/Box.hpp"
#include "art/geometry/Cylinder.hpp"
#include "art/geometry/Disk.hpp"
#include "art/geometry/MeshGeometry.hpp"
#include "art/geometry/Plane.hpp"
#include "art/geometry/Sphere.hpp"
#include "art/geometry/Triangle.hpp"
#include "art/geometry/mesh_utils.hpp"

namespace art::lua {

void bind_geometry(sol::table &tbl) {
    // ── Sphere ──
    tbl.new_usertype<geometry::Sphere>(
        "Sphere", sol::call_constructor, sol::factories([] {
            return std::make_shared<geometry::Sphere>();
        }));

    // ── Box ──
    tbl.new_usertype<geometry::Box>(
        "Box",
        sol::call_constructor,
        sol::factories([] { return std::make_shared<geometry::Box>(); },
                       [](const sol::table &min_t, const sol::table &max_t) {
                           auto mn = table_to_vec3(min_t);
                           auto mx = table_to_vec3(max_t);
                           return std::make_shared<geometry::Box>(Point(mn),
                                                                  Point(mx));
                       }),
        "min",
        [](const geometry::Box &b, sol::this_state ts) {
            Vector3d v = b.min();
            return vec3_to_table(sol::state_view(ts), v);
        },
        "max",
        [](const geometry::Box &b, sol::this_state ts) {
            Vector3d v = b.max();
            return vec3_to_table(sol::state_view(ts), v);
        });

    // ── Plane ──
    tbl.new_usertype<geometry::Plane>(
        "Plane", sol::call_constructor, sol::factories([] {
            return std::make_shared<geometry::Plane>();
        }));

    // ── Disk ──
    tbl.new_usertype<geometry::Disk>(
        "Disk", sol::call_constructor, sol::factories([] {
            return std::make_shared<geometry::Disk>();
        }));

    // ── Cylinder ──
    tbl.new_usertype<geometry::Cylinder>(
        "Cylinder", sol::call_constructor, sol::factories([] {
            return std::make_shared<geometry::Cylinder>();
        }));

    // ── Triangle ──
    tbl.new_usertype<geometry::Triangle>(
        "Triangle",
        sol::call_constructor,
        sol::factories([](const sol::table &v0t,
                          const sol::table &v1t,
                          const sol::table &v2t) {
            return std::make_shared<geometry::Triangle>(
                table_to_vec3(v0t), table_to_vec3(v1t), table_to_vec3(v2t));
        }));

    // ── MeshGeometry ──
    tbl.new_usertype<geometry::MeshGeometry>(
        "MeshGeometry",
        sol::no_constructor,

        "triangle_count",
        &geometry::MeshGeometry::triangle_count);

    // ── Factory functions ──
    tbl.set_function("make_cube_mesh",
                     [] { return geometry::make_cube_mesh(); });

    tbl.set_function("make_tetrahedron_mesh",
                     [] { return geometry::make_tetrahedron_mesh(); });

    tbl.set_function(
        "make_mesh",
        [](const sol::table &verts_t, const sol::table &tris_t)
            -> std::shared_ptr<geometry::MeshGeometry> {
            using Vec3 = std::array<double, 3>;
            using Tri = std::array<int64_t, 3>;

            std::vector<Vec3> vertices;
            vertices.reserve(verts_t.size());
            for (size_t i = 1; i <= verts_t.size(); ++i) {
                sol::table v = verts_t[i];
                vertices.push_back(
                    {v.get_or(1, 0.0), v.get_or(2, 0.0), v.get_or(3, 0.0)});
            }

            std::vector<Tri> triangles;
            triangles.reserve(tris_t.size());
            for (size_t i = 1; i <= tris_t.size(); ++i) {
                sol::table t = tris_t[i];
                triangles.push_back({t.get_or<int64_t>(1, 0),
                                     t.get_or<int64_t>(2, 0),
                                     t.get_or<int64_t>(3, 0)});
            }

            return geometry::make_mesh_geometry(vertices, triangles);
        });
}

} // namespace art::lua
