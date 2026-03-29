#include <catch2/catch_all.hpp>

#include "art/Ray.hpp"
#include "art/geometry/MeshGeometry.hpp"
#include "art/geometry/mesh_utils.hpp"

using namespace art;
using namespace art::geometry;

TEST_CASE("mesh_geometry_single_triangle_hit", "[mesh][geometry]") {
    // z=0 triangle matching the analytic Triangle test
    auto mesh = make_single_triangle_mesh(
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0});

    Ray ray{Point(0.25, 0.25, 2.0), Vector3d{0.0, 0.0, -1.0}};
    std::optional<Intersection> isect;
    bool hit = mesh->intersect(ray, isect);

    REQUIRE(hit);
    REQUIRE(isect.has_value());
    CHECK(double(isect->t) == Catch::Approx(2.0).margin(1e-6));

    // Hit point should be at (0.25, 0.25, 0)
    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(0.25).margin(1e-6));
    CHECK(pos(1) == Catch::Approx(0.25).margin(1e-6));
    CHECK(pos(2) == Catch::Approx(0.0).margin(1e-6));

    // Normal should point in +z or -z direction
    double nz = isect->geometric_normal(2);
    CHECK(std::abs(nz) > 0.0);

    // Geometry pointer should be set
    CHECK(isect->geometry == mesh.get());
}

TEST_CASE("mesh_geometry_single_triangle_miss", "[mesh][geometry]") {
    auto mesh = make_single_triangle_mesh(
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0});

    // Ray misses the triangle entirely
    Ray ray{Point(5.0, 5.0, 2.0), Vector3d{0.0, 0.0, -1.0}};
    std::optional<Intersection> isect;
    bool hit = mesh->intersect(ray, isect);

    CHECK_FALSE(hit);
}

TEST_CASE("mesh_geometry_cube_hit_faces", "[mesh][geometry]") {
    auto cube = make_cube_mesh();

    // Cast ray at +z face from above
    Ray ray{Point(0.0, 0.0, 2.0), Vector3d{0.0, 0.0, -1.0}};
    std::optional<Intersection> isect;
    bool hit = cube->intersect(ray, isect);

    REQUIRE(hit);
    REQUIRE(isect.has_value());
    // Should hit the +z face at z=0.5, so t = 2.0 - 0.5 = 1.5
    CHECK(double(isect->t) == Catch::Approx(1.5).margin(1e-6));

    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(2) == Catch::Approx(0.5).margin(1e-6));
}

TEST_CASE("mesh_geometry_cube_hit_negative_face", "[mesh][geometry]") {
    auto cube = make_cube_mesh();

    // Cast ray at -x face from the left
    Ray ray{Point(-2.0, 0.0, 0.0), Vector3d{1.0, 0.0, 0.0}};
    std::optional<Intersection> isect;
    bool hit = cube->intersect(ray, isect);

    REQUIRE(hit);
    REQUIRE(isect.has_value());
    // Should hit the -x face at x=-0.5, so t = 2.0 - 0.5 = 1.5
    CHECK(double(isect->t) == Catch::Approx(1.5).margin(1e-6));

    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(-0.5).margin(1e-6));
}

TEST_CASE("mesh_geometry_tmax_pruning", "[mesh][geometry]") {
    auto mesh = make_single_triangle_mesh(
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0});

    // Set tMax so the triangle is too far
    Ray ray{Point(0.25, 0.25, 2.0), Vector3d{0.0, 0.0, -1.0}};
    ray.tMax = Rational(1.0); // triangle at t=2, but tMax=1

    std::optional<Intersection> isect;
    bool hit = mesh->intersect(ray, isect);

    CHECK_FALSE(hit);
}

TEST_CASE("mesh_geometry_tmax_updates_on_hit", "[mesh][geometry]") {
    auto mesh = make_single_triangle_mesh(
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0});

    Ray ray{Point(0.25, 0.25, 2.0), Vector3d{0.0, 0.0, -1.0}};
    std::optional<Intersection> isect;
    bool hit = mesh->intersect(ray, isect);

    REQUIRE(hit);
    // tMax should be updated to the hit distance
    CHECK(double(ray.tMax) == Catch::Approx(2.0).margin(1e-6));
}

TEST_CASE("mesh_geometry_tetrahedron_construction", "[mesh][geometry]") {
    auto tet = make_tetrahedron_mesh();
    CHECK(tet->triangle_count() == 4);
}

TEST_CASE("mesh_geometry_cube_triangle_count", "[mesh][geometry]") {
    auto cube = make_cube_mesh();
    CHECK(cube->triangle_count() == 12);
}

TEST_CASE("mesh_geometry_bounding_box", "[mesh][geometry]") {
    auto mesh = make_single_triangle_mesh(
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0});

    auto bbox = mesh->bounding_box();
    Vector3d lo = Vector3d(bbox.min());
    Vector3d hi = Vector3d(bbox.max());

    CHECK(lo(0) <= 0.0);
    CHECK(lo(1) <= 0.0);
    CHECK(lo(2) <= 0.0);
    CHECK(hi(0) >= 1.0);
    CHECK(hi(1) >= 1.0);
    CHECK(hi(2) >= 0.0);
}
