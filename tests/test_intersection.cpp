
#include <catch2/catch_all.hpp>
#include <cmath>
#include <numbers>

#include <art/Point.hpp>
#include <art/Ray.hpp>
#include <art/geometry/Box.hpp>
#include <art/geometry/Sphere.hpp>
#include <art/objects/InternalSceneNode.hpp>
#include <art/objects/Object.hpp>

#include <zipper/transform/transform.hpp>

using namespace art;

// ============================================================
// Sphere intersection — UV, tangent vectors, geometry pointer
// ============================================================

TEST_CASE("sphere_intersection_populates_all_fields",
          "[sphere][intersection]") {
    auto sphere = std::make_shared<geometry::Sphere>();
    Ray ray;
    ray.origin = Point(0, 0, 3);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    REQUIRE(sphere->intersect(ray, isect));
    REQUIRE(isect.has_value());

    // Hit point should be on the +z pole of the unit sphere
    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(0.0).margin(1e-10));
    CHECK(pos(1) == Catch::Approx(0.0).margin(1e-10));
    CHECK(pos(2) == Catch::Approx(1.0).margin(1e-10));

    // Geometric normal at +z pole should point in +z direction
    auto n = isect->geometric_normal;
    CHECK(n(2) > 0.0);
    CHECK(std::abs(n(0)) < 1e-10);
    CHECK(std::abs(n(1)) < 1e-10);

    // UV at +z pole: theta=0, so v should be ~0
    CHECK(isect->uv(1) == Catch::Approx(0.0).margin(1e-6));

    // Geometry pointer should be set
    CHECK(isect->geometry
          == static_cast<const geometry::Geometry *>(sphere.get()));
    CHECK(isect->primitive_index == 0);
}

TEST_CASE("sphere_intersection_uv_equator", "[sphere][intersection]") {
    auto sphere = std::make_shared<geometry::Sphere>();
    // Ray hitting equator at +x
    Ray ray;
    ray.origin = Point(3, 0, 0);
    ray.direction = Vector3d{-1.0, 0.0, 0.0};

    std::optional<Intersection> isect;
    REQUIRE(sphere->intersect(ray, isect));
    REQUIRE(isect.has_value());

    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(1.0).margin(1e-10));
    CHECK(pos(1) == Catch::Approx(0.0).margin(1e-10));
    CHECK(pos(2) == Catch::Approx(0.0).margin(1e-10));

    // At equator (+x, 0, 0): phi=0, theta=pi/2
    // u = phi/(2pi) = 0, v = theta/pi = 0.5
    CHECK(isect->uv(0) == Catch::Approx(0.0).margin(1e-6));
    CHECK(isect->uv(1) == Catch::Approx(0.5).margin(1e-6));

    // dpdu at equator (+x): (-y, x, 0) = (0, 1, 0)
    auto dpdu_norm = isect->dpdu.normalized();
    CHECK(std::abs(dpdu_norm(0)) < 1e-6);
    CHECK(dpdu_norm(1) == Catch::Approx(1.0).margin(1e-6));
    CHECK(std::abs(dpdu_norm(2)) < 1e-6);
}

TEST_CASE("sphere_miss_returns_false", "[sphere][intersection]") {
    auto sphere = std::make_shared<geometry::Sphere>();
    Ray ray;
    ray.origin = Point(0, 5, 0);
    ray.direction = Vector3d{1.0, 0.0, 0.0}; // parallel, misses

    std::optional<Intersection> isect;
    CHECK_FALSE(sphere->intersect(ray, isect));
    CHECK_FALSE(isect.has_value());
}

// ============================================================
// Box intersection — UV, tangent vectors, face index
// ============================================================

TEST_CASE("box_intersection_populates_all_fields", "[box][intersection]") {
    auto box = std::make_shared<geometry::Box>();
    Ray ray;
    ray.origin = Point(0, 0, 3);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    REQUIRE(box->intersect(ray, isect));
    REQUIRE(isect.has_value());

    // Should hit the +z face of the unit box (z = 0.5)
    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(2) == Catch::Approx(0.5).margin(1e-10));

    // Normal should point in +z (from_above = true for z axis)
    auto n = isect->geometric_normal;
    CHECK(n(2) == Catch::Approx(1.0).margin(1e-10));
    CHECK(std::abs(n(0)) < 1e-10);
    CHECK(std::abs(n(1)) < 1e-10);

    // UV should be in [0,1] range
    CHECK(isect->uv(0) >= 0.0);
    CHECK(isect->uv(0) <= 1.0);
    CHECK(isect->uv(1) >= 0.0);
    CHECK(isect->uv(1) <= 1.0);

    // dpdu/dpdv should be axis-aligned unit vectors on the face plane
    double dpdu_len = double(isect->dpdu.norm<2>());
    double dpdv_len = double(isect->dpdv.norm<2>());
    CHECK(dpdu_len == Catch::Approx(1.0).margin(1e-10));
    CHECK(dpdv_len == Catch::Approx(1.0).margin(1e-10));

    // Geometry pointer
    CHECK(isect->geometry
          == static_cast<const geometry::Geometry *>(box.get()));
    // Face index for +z face with from_above=true should be 2
    CHECK(isect->primitive_index == 2);
}

TEST_CASE("box_intersection_negative_face", "[box][intersection]") {
    auto box = std::make_shared<geometry::Box>();
    // Ray hitting the -x face from below
    Ray ray;
    ray.origin = Point(-3, 0, 0);
    ray.direction = Vector3d{1.0, 0.0, 0.0};

    std::optional<Intersection> isect;
    REQUIRE(box->intersect(ray, isect));
    REQUIRE(isect.has_value());

    // Normal should point in -x (from_above = false)
    auto n = isect->geometric_normal;
    CHECK(n(0) == Catch::Approx(-1.0).margin(1e-10));

    // Primitive index for -x face: axis 0 + 3 = 3
    CHECK(isect->primitive_index == 3);
}

// ============================================================
// tMax pruning
// ============================================================

TEST_CASE("tmax_prunes_farther_intersections", "[tmax][intersection]") {
    auto sphere = std::make_shared<geometry::Sphere>();
    Ray ray;
    ray.origin = Point(0, 0, 5);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    // Set tMax so the sphere (at distance ~4) is beyond it
    ray.tMax = Rational(2.0);

    std::optional<Intersection> isect;
    CHECK_FALSE(sphere->intersect(ray, isect));
    CHECK_FALSE(isect.has_value());
}

TEST_CASE("tmax_accepts_closer_intersections", "[tmax][intersection]") {
    auto sphere = std::make_shared<geometry::Sphere>();
    Ray ray;
    ray.origin = Point(0, 0, 5);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    // Set tMax so the sphere (at distance ~4) is within it
    ray.tMax = Rational(10.0);

    std::optional<Intersection> isect;
    REQUIRE(sphere->intersect(ray, isect));
    REQUIRE(isect.has_value());

    // After hit, tMax should be updated to the hit distance
    double tmax_val = double(ray.tMax);
    CHECK(tmax_val == Catch::Approx(4.0).margin(1e-6));
}

TEST_CASE("tmax_closest_hit_with_direct_geometry", "[tmax][intersection]") {
    // Test tMax closest-hit semantics by intersecting two geometries directly
    auto sphere = std::make_shared<geometry::Sphere>();

    Ray ray;
    ray.origin = Point(0, 0, 5);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    // First intersection: sphere at origin, hit at t~4
    std::optional<Intersection> isect;
    REQUIRE(sphere->intersect(ray, isect));
    double t_first = double(ray.tMax);
    CHECK(t_first == Catch::Approx(4.0).margin(1e-6));

    // Second intersection attempt with a box that is farther away
    // The box at default [-0.5, 0.5]^3 would be hit at t~4.5, which is > tMax
    // So this should fail
    auto box = std::make_shared<geometry::Box>();
    // tMax is already 4.0 from the sphere hit, box face at z=0.5 is at t=4.5
    CHECK_FALSE(box->intersect(ray, isect));

    // tMax should remain at the sphere hit distance
    CHECK(double(ray.tMax) == Catch::Approx(4.0).margin(1e-6));
}

// ============================================================
// SceneNode normal/tangent transforms
// ============================================================

TEST_CASE("scene_node_transforms_normals_and_tangents",
          "[scenenode][intersection]") {
    auto sphere = std::make_shared<geometry::Sphere>();
    auto obj = std::make_shared<objects::Object>(*sphere);

    // Translate the sphere to (3, 0, 0)
    obj->transform() =
        zipper::transform::Translation<double>(Vector3d{3.0, 0.0, 0.0})
            .to_transform();
    obj->update_bounding_box();

    Ray ray;
    ray.origin = Point(10, 0, 0);
    ray.direction = Vector3d{-1.0, 0.0, 0.0};

    std::optional<Intersection> isect;
    REQUIRE(obj->intersect(ray, isect));
    REQUIRE(isect.has_value());

    // Hit point should be near (4, 0, 0) — surface of unit sphere at origin
    // translated by (3,0,0)
    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(4.0).margin(1e-6));
    CHECK(std::abs(pos(1)) < 1e-6);
    CHECK(std::abs(pos(2)) < 1e-6);

    // Normal should point in +x direction (outward from sphere center at
    // (3,0,0))
    auto n = isect->geometric_normal.normalized();
    CHECK(n(0) == Catch::Approx(1.0).margin(1e-4));
    CHECK(std::abs(n(1)) < 1e-4);
    CHECK(std::abs(n(2)) < 1e-4);
}

TEST_CASE("scene_node_scaled_normals", "[scenenode][intersection]") {
    auto sphere = std::make_shared<geometry::Sphere>();
    auto obj = std::make_shared<objects::Object>(*sphere);

    // Non-uniform scale: stretch x by 2
    auto S = zipper::transform::Scaling<double>(Vector3d{2.0, 1.0, 1.0});
    obj->transform() = S.to_transform();
    obj->update_bounding_box();

    // Ray along x axis at the equator
    Ray ray;
    ray.origin = Point(10, 0, 0);
    ray.direction = Vector3d{-1.0, 0.0, 0.0};

    std::optional<Intersection> isect;
    REQUIRE(obj->intersect(ray, isect));
    REQUIRE(isect.has_value());

    // Hit point should be near (2, 0, 0) — scaled sphere surface
    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(2.0).margin(1e-4));

    // For non-uniform scale, the normal should still point in +x
    // (inverse-transpose of diagonal scale preserves axis alignment for
    // axis-aligned hit)
    auto n = isect->geometric_normal.normalized();
    CHECK(n(0) == Catch::Approx(1.0).margin(1e-4));
}
