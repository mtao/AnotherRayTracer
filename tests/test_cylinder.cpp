
#include <catch2/catch_all.hpp>
#include <cmath>
#include <numbers>

#include <art/Point.hpp>
#include <art/Ray.hpp>
#include <art/geometry/Cylinder.hpp>

using namespace art;

TEST_CASE("cylinder_hit_along_x", "[cylinder][intersection]") {
    auto cyl = std::make_shared<geometry::Cylinder>();
    // Ray from outside along -x, hitting the cylinder at x=1
    Ray ray;
    ray.origin = Point(3, 0, 0.5);
    ray.direction = Vector3d{-1.0, 0.0, 0.0};

    std::optional<Intersection> isect;
    REQUIRE(cyl->intersect(ray, isect));
    REQUIRE(isect.has_value());

    // Hit at x=1, y=0, z=0.5 -> t=2
    CHECK(double(isect->t) == Catch::Approx(2.0).margin(1e-6));

    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(1.0).margin(1e-6));
    CHECK(pos(1) == Catch::Approx(0.0).margin(1e-6));
    CHECK(pos(2) == Catch::Approx(0.5).margin(1e-6));

    // Normal should point in +x (radial outward)
    auto n = isect->geometric_normal.normalized();
    CHECK(n(0) == Catch::Approx(1.0).margin(1e-6));
    CHECK(std::abs(n(1)) < 1e-6);
    CHECK(std::abs(n(2)) < 1e-6);

    // UV: theta=0, so u = 0; z = 0.5
    CHECK(isect->uv(0) == Catch::Approx(0.0).margin(1e-6));
    CHECK(isect->uv(1) == Catch::Approx(0.5).margin(1e-6));

    CHECK(isect->geometry
          == static_cast<const geometry::Geometry *>(cyl.get()));
}

TEST_CASE("cylinder_miss_above_z_range", "[cylinder][intersection]") {
    auto cyl = std::make_shared<geometry::Cylinder>();
    // Ray at z=2, well above the cylinder's z=[0,1] range
    Ray ray;
    ray.origin = Point(3, 0, 2);
    ray.direction = Vector3d{-1.0, 0.0, 0.0};

    std::optional<Intersection> isect;
    CHECK_FALSE(cyl->intersect(ray, isect));
}

TEST_CASE("cylinder_miss_below_z_range", "[cylinder][intersection]") {
    auto cyl = std::make_shared<geometry::Cylinder>();
    // Ray at z=-1, below the cylinder
    Ray ray;
    ray.origin = Point(3, 0, -1);
    ray.direction = Vector3d{-1.0, 0.0, 0.0};

    std::optional<Intersection> isect;
    CHECK_FALSE(cyl->intersect(ray, isect));
}

TEST_CASE("cylinder_miss_parallel_to_axis", "[cylinder][intersection]") {
    auto cyl = std::make_shared<geometry::Cylinder>();
    // Ray parallel to z-axis but outside cylinder radius
    Ray ray;
    ray.origin = Point(2, 0, -1);
    ray.direction = Vector3d{0.0, 0.0, 1.0};

    std::optional<Intersection> isect;
    CHECK_FALSE(cyl->intersect(ray, isect));
}

TEST_CASE("cylinder_hit_tangent_vectors", "[cylinder][intersection]") {
    auto cyl = std::make_shared<geometry::Cylinder>();
    Ray ray;
    ray.origin = Point(0, 3, 0.5);
    ray.direction = Vector3d{0.0, -1.0, 0.0};

    std::optional<Intersection> isect;
    REQUIRE(cyl->intersect(ray, isect));

    // dpdu should be tangent along circumference (at y=1: dpdu = (-y, x, 0) =
    // (-1, 0, 0)) Wait, at hit point (0, 1, 0.5): dpdu = (-1, 0, 0)
    auto dpdu_n = isect->dpdu.normalized();
    CHECK(dpdu_n(0) == Catch::Approx(-1.0).margin(1e-4));

    // dpdv should be along z-axis
    CHECK(isect->dpdv(2) == Catch::Approx(1.0).margin(1e-10));
}

TEST_CASE("cylinder_tmax_respected", "[cylinder][tmax]") {
    auto cyl = std::make_shared<geometry::Cylinder>();
    Ray ray;
    ray.origin = Point(3, 0, 0.5);
    ray.direction = Vector3d{-1.0, 0.0, 0.0};
    ray.tMax = Rational(1.0); // cylinder hit at t=2, but tMax=1

    std::optional<Intersection> isect;
    CHECK_FALSE(cyl->intersect(ray, isect));
}
