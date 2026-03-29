
#include <catch2/catch_all.hpp>
#include <cmath>
#include <numbers>

#include <art/Point.hpp>
#include <art/Ray.hpp>
#include <art/geometry/Disk.hpp>

using namespace art;

TEST_CASE("disk_hit_center", "[disk][intersection]") {
    auto disk = std::make_shared<geometry::Disk>();
    Ray ray;
    ray.origin = Point(0, 0, 2);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    REQUIRE(disk->intersect(ray, isect));
    REQUIRE(isect.has_value());

    CHECK(double(isect->t) == Catch::Approx(2.0).margin(1e-10));

    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(0.0).margin(1e-10));
    CHECK(pos(1) == Catch::Approx(0.0).margin(1e-10));
    CHECK(pos(2) == Catch::Approx(0.0).margin(1e-6));

    // Normal always (0, 0, 1)
    CHECK(isect->geometric_normal(2) == Catch::Approx(1.0));

    // UV at center: r=0, phi=undefined but consistent
    CHECK(isect->uv(0) == Catch::Approx(0.0).margin(1e-10));

    CHECK(isect->geometry
          == static_cast<const geometry::Geometry *>(disk.get()));
}

TEST_CASE("disk_hit_edge", "[disk][intersection]") {
    auto disk = std::make_shared<geometry::Disk>();
    // Ray aimed at edge of disk at (0.9, 0, 0)
    Ray ray;
    ray.origin = Point(0.9, 0, 3);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    REQUIRE(disk->intersect(ray, isect));
    REQUIRE(isect.has_value());

    // UV: r should be ~0.9
    CHECK(isect->uv(0) == Catch::Approx(0.9).margin(1e-6));
}

TEST_CASE("disk_miss_outside_radius", "[disk][intersection]") {
    auto disk = std::make_shared<geometry::Disk>();
    // Ray aimed outside the disk at (1.5, 0, 0)
    Ray ray;
    ray.origin = Point(1.5, 0, 3);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    CHECK_FALSE(disk->intersect(ray, isect));
}

TEST_CASE("disk_parallel_misses", "[disk][intersection]") {
    auto disk = std::make_shared<geometry::Disk>();
    Ray ray;
    ray.origin = Point(0, 0, 0);
    ray.direction = Vector3d{1.0, 0.0, 0.0};

    std::optional<Intersection> isect;
    CHECK_FALSE(disk->intersect(ray, isect));
}

TEST_CASE("disk_tmax_respected", "[disk][tmax]") {
    auto disk = std::make_shared<geometry::Disk>();
    Ray ray;
    ray.origin = Point(0, 0, 5);
    ray.direction = Vector3d{0.0, 0.0, -1.0};
    ray.tMax = Rational(2.0);

    std::optional<Intersection> isect;
    CHECK_FALSE(disk->intersect(ray, isect));
}
