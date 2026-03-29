
#include <catch2/catch_all.hpp>
#include <cmath>

#include <art/Point.hpp>
#include <art/Ray.hpp>
#include <art/geometry/Plane.hpp>

using namespace art;

TEST_CASE("plane_hit_from_above", "[plane][intersection]") {
    auto plane = std::make_shared<geometry::Plane>();
    Ray ray;
    ray.origin = Point(0, 0, 3);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    REQUIRE(plane->intersect(ray, isect));
    REQUIRE(isect.has_value());

    // Hit at z=0, so t should be 3
    CHECK(double(isect->t) == Catch::Approx(3.0).margin(1e-10));

    // Position should be at (0, 0, 0)
    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(0.0).margin(1e-10));
    CHECK(pos(1) == Catch::Approx(0.0).margin(1e-10));
    CHECK(pos(2) == Catch::Approx(0.0).margin(1e-6));

    // Normal always (0, 0, 1)
    CHECK(isect->geometric_normal(2) == Catch::Approx(1.0).margin(1e-10));

    // dpdu/dpdv are axis-aligned
    CHECK(isect->dpdu(0) == Catch::Approx(1.0).margin(1e-10));
    CHECK(isect->dpdv(1) == Catch::Approx(1.0).margin(1e-10));

    CHECK(isect->geometry
          == static_cast<const geometry::Geometry *>(plane.get()));
}

TEST_CASE("plane_hit_from_below", "[plane][intersection]") {
    auto plane = std::make_shared<geometry::Plane>();
    Ray ray;
    ray.origin = Point(1, 2, -5);
    ray.direction = Vector3d{0.0, 0.0, 1.0};

    std::optional<Intersection> isect;
    REQUIRE(plane->intersect(ray, isect));
    REQUIRE(isect.has_value());

    CHECK(double(isect->t) == Catch::Approx(5.0).margin(1e-10));

    // UV should be (x, y) of hit point = (1, 2)
    CHECK(isect->uv(0) == Catch::Approx(1.0).margin(1e-6));
    CHECK(isect->uv(1) == Catch::Approx(2.0).margin(1e-6));
}

TEST_CASE("plane_parallel_ray_misses", "[plane][intersection]") {
    auto plane = std::make_shared<geometry::Plane>();
    Ray ray;
    ray.origin = Point(0, 0, 1);
    ray.direction = Vector3d{1.0, 0.0, 0.0}; // parallel to plane

    std::optional<Intersection> isect;
    CHECK_FALSE(plane->intersect(ray, isect));
}

TEST_CASE("plane_ray_behind_misses", "[plane][intersection]") {
    auto plane = std::make_shared<geometry::Plane>();
    Ray ray;
    ray.origin = Point(0, 0, -1);
    ray.direction = Vector3d{0.0, 0.0, -1.0}; // going away from plane

    std::optional<Intersection> isect;
    CHECK_FALSE(plane->intersect(ray, isect));
}

TEST_CASE("plane_tmax_respected", "[plane][tmax]") {
    auto plane = std::make_shared<geometry::Plane>();
    Ray ray;
    ray.origin = Point(0, 0, 5);
    ray.direction = Vector3d{0.0, 0.0, -1.0};
    ray.tMax = Rational(2.0); // plane at t=5, but tMax=2

    std::optional<Intersection> isect;
    CHECK_FALSE(plane->intersect(ray, isect));
}
