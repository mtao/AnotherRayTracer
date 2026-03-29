
#include <catch2/catch_all.hpp>
#include <cmath>

#include <art/Point.hpp>
#include <art/Ray.hpp>
#include <art/geometry/Triangle.hpp>

using namespace art;

TEST_CASE("triangle_hit_center", "[triangle][intersection]") {
    auto tri = std::make_shared<geometry::Triangle>(Vector3d{0.0, 0.0, 0.0},
                                                    Vector3d{1.0, 0.0, 0.0},
                                                    Vector3d{0.0, 1.0, 0.0});

    // Ray from above aimed at triangle centroid
    Ray ray;
    ray.origin = Point(0.25, 0.25, 2);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    REQUIRE(tri->intersect(ray, isect));
    REQUIRE(isect.has_value());

    CHECK(double(isect->t) == Catch::Approx(2.0).margin(1e-6));

    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(0.25).margin(1e-6));
    CHECK(pos(1) == Catch::Approx(0.25).margin(1e-6));
    CHECK(pos(2) == Catch::Approx(0.0).margin(1e-6));

    // Normal should be in +z direction (edge1 x edge2 = (1,0,0) x (0,1,0) =
    // (0,0,1))
    auto n = isect->geometric_normal.normalized();
    CHECK(n(2) == Catch::Approx(1.0).margin(1e-6));

    // UV should be barycentric (u, v) for v1 and v2
    CHECK(isect->uv(0) >= 0.0);
    CHECK(isect->uv(1) >= 0.0);
    CHECK(isect->uv(0) + isect->uv(1) <= 1.0);

    // dpdu = edge1, dpdv = edge2 (no custom UVs)
    CHECK(isect->dpdu(0) == Catch::Approx(1.0).margin(1e-10));
    CHECK(isect->dpdv(1) == Catch::Approx(1.0).margin(1e-10));

    CHECK(isect->geometry
          == static_cast<const geometry::Geometry *>(tri.get()));
}

TEST_CASE("triangle_miss_outside", "[triangle][intersection]") {
    auto tri = std::make_shared<geometry::Triangle>(Vector3d{0.0, 0.0, 0.0},
                                                    Vector3d{1.0, 0.0, 0.0},
                                                    Vector3d{0.0, 1.0, 0.0});

    // Ray aimed outside the triangle
    Ray ray;
    ray.origin = Point(2, 2, 2);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    CHECK_FALSE(tri->intersect(ray, isect));
}

TEST_CASE("triangle_miss_parallel", "[triangle][intersection]") {
    auto tri = std::make_shared<geometry::Triangle>(Vector3d{0.0, 0.0, 0.0},
                                                    Vector3d{1.0, 0.0, 0.0},
                                                    Vector3d{0.0, 1.0, 0.0});

    // Ray parallel to triangle
    Ray ray;
    ray.origin = Point(0, 0, 1);
    ray.direction = Vector3d{1.0, 0.0, 0.0};

    std::optional<Intersection> isect;
    CHECK_FALSE(tri->intersect(ray, isect));
}

TEST_CASE("triangle_hit_vertex", "[triangle][intersection]") {
    auto tri = std::make_shared<geometry::Triangle>(Vector3d{0.0, 0.0, 0.0},
                                                    Vector3d{1.0, 0.0, 0.0},
                                                    Vector3d{0.0, 1.0, 0.0});

    // Ray aimed at v0 = (0, 0, 0) — just slightly inside
    Ray ray;
    ray.origin = Point(0.001, 0.001, 2);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    REQUIRE(tri->intersect(ray, isect));
}

TEST_CASE("triangle_with_custom_uvs", "[triangle][intersection]") {
    auto tri = std::make_shared<geometry::Triangle>(Vector3d{0.0, 0.0, 0.0},
                                                    Vector3d{1.0, 0.0, 0.0},
                                                    Vector3d{0.0, 1.0, 0.0},
                                                    Vector2d{0.0, 0.0},
                                                    Vector2d{1.0, 0.0},
                                                    Vector2d{0.0, 1.0});

    // Ray aimed at centroid
    Ray ray;
    ray.origin = Point(0.25, 0.25, 2);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect;
    REQUIRE(tri->intersect(ray, isect));

    // UV should be interpolated from vertex UVs
    CHECK(isect->uv(0) == Catch::Approx(0.25).margin(1e-4));
    CHECK(isect->uv(1) == Catch::Approx(0.25).margin(1e-4));
}

TEST_CASE("triangle_tmax_respected", "[triangle][tmax]") {
    auto tri = std::make_shared<geometry::Triangle>(Vector3d{0.0, 0.0, 0.0},
                                                    Vector3d{1.0, 0.0, 0.0},
                                                    Vector3d{0.0, 1.0, 0.0});

    Ray ray;
    ray.origin = Point(0.25, 0.25, 5);
    ray.direction = Vector3d{0.0, 0.0, -1.0};
    ray.tMax = Rational(2.0); // triangle at t=5, but tMax=2

    std::optional<Intersection> isect;
    CHECK_FALSE(tri->intersect(ray, isect));
}

TEST_CASE("triangle_bounding_box", "[triangle][bbox]") {
    auto tri = std::make_shared<geometry::Triangle>(Vector3d{-1.0, -2.0, 0.0},
                                                    Vector3d{3.0, 0.0, 1.0},
                                                    Vector3d{0.0, 4.0, -1.0});

    auto bb = tri->bounding_box();
    Vector3d lo = Vector3d(bb.min());
    Vector3d hi = Vector3d(bb.max());

    CHECK(lo(0) == Catch::Approx(-1.0));
    CHECK(lo(1) == Catch::Approx(-2.0));
    CHECK(lo(2) == Catch::Approx(-1.0));
    CHECK(hi(0) == Catch::Approx(3.0));
    CHECK(hi(1) == Catch::Approx(4.0));
    CHECK(hi(2) == Catch::Approx(1.0));
}
