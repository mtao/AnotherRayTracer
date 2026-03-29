
#include <catch2/catch_all.hpp>
#include <cmath>

#include <art/Point.hpp>
#include <art/Ray.hpp>
#include <art/accel/BVHAccelerator.hpp>
#include <art/accel/LinearAccelerator.hpp>
#include <art/geometry/Box.hpp>
#include <art/geometry/Sphere.hpp>
#include <art/objects/InternalSceneNode.hpp>
#include <art/objects/Object.hpp>

#include <zipper/transform/transform.hpp>

using namespace art;

// Helper: build a multi-object scene with transforms
static auto make_test_scene() -> std::shared_ptr<objects::InternalSceneNode> {
    auto scene = objects::InternalSceneNode::create();

    // Sphere translated to (0, -1.5, 0)
    auto sphere_obj = std::make_shared<objects::Object>(
        *std::make_shared<geometry::Sphere>());
    sphere_obj->transform() =
        zipper::transform::Translation<double>(Vector3d{0.0, -1.5, 0.0})
            .to_transform();
    scene->add_node(sphere_obj);

    // Box translated to (0, 1.5, 0)
    auto box_obj =
        std::make_shared<objects::Object>(*std::make_shared<geometry::Box>());
    box_obj->transform() =
        zipper::transform::Translation<double>(Vector3d{0.0, 1.5, 0.0})
            .to_transform();
    scene->add_node(box_obj);

    // Rotated box at origin
    auto rotated_box =
        std::make_shared<objects::Object>(*std::make_shared<geometry::Box>());
    rotated_box->transform() =
        zipper::transform::AxisAngleRotation<double>(0.7, {0., 0., 1.})
            .to_transform();
    scene->add_node(rotated_box);

    return scene;
}

// ============================================================
// Flattening
// ============================================================

TEST_CASE("accelerator_flattens_scene_graph", "[accelerator]") {
    auto scene = make_test_scene();

    accel::LinearAccelerator accel;
    accel.build(*scene);

    // 3 leaf objects -> 3 primitives
    CHECK(accel.primitives().size() == 3);
}

// ============================================================
// Linear vs BVH agreement
// ============================================================

TEST_CASE("linear_and_bvh_agree_on_hits", "[accelerator]") {
    auto scene = make_test_scene();

    accel::LinearAccelerator linear;
    linear.build(*scene);

    accel::BVHAccelerator bvh;
    bvh.build(*scene);

    // Fire a grid of rays at the scene and verify both accelerators agree
    for (int iy = -5; iy <= 5; ++iy) {
        for (int ix = -5; ix <= 5; ++ix) {
            double x = ix * 0.5;
            double y = iy * 0.5;

            Ray ray_lin;
            ray_lin.origin = Point(x, y, 10);
            ray_lin.direction = Vector3d{0.0, 0.0, -1.0};

            Ray ray_bvh;
            ray_bvh.origin = Point(x, y, 10);
            ray_bvh.direction = Vector3d{0.0, 0.0, -1.0};

            std::optional<Intersection> isect_lin;
            std::optional<Intersection> isect_bvh;

            bool hit_lin = linear.intersect(ray_lin, isect_lin);
            bool hit_bvh = bvh.intersect(ray_bvh, isect_bvh);

            CAPTURE(x, y);
            CHECK(hit_lin == hit_bvh);

            if (hit_lin && hit_bvh) {
                // tMax should agree (closest hit distance)
                CHECK(double(ray_lin.tMax)
                      == Catch::Approx(double(ray_bvh.tMax)).margin(1e-6));

                // Hit positions should agree
                Vector3d pos_lin = Vector3d(isect_lin->position);
                Vector3d pos_bvh = Vector3d(isect_bvh->position);
                for (int d = 0; d < 3; ++d) {
                    CHECK(pos_lin(d) == Catch::Approx(pos_bvh(d)).margin(1e-6));
                }

                // Normals should agree
                auto n_lin = isect_lin->geometric_normal.normalized();
                auto n_bvh = isect_bvh->geometric_normal.normalized();
                for (int d = 0; d < 3; ++d) {
                    CHECK(n_lin(d) == Catch::Approx(n_bvh(d)).margin(1e-4));
                }
            }
        }
    }
}

// ============================================================
// Single-object scenes
// ============================================================

TEST_CASE("bvh_single_sphere", "[accelerator][bvh]") {
    auto obj = std::make_shared<objects::Object>(
        *std::make_shared<geometry::Sphere>());

    accel::BVHAccelerator bvh;
    bvh.build(*obj);

    // Hit
    Ray ray;
    ray.origin = Point(0, 0, 5);
    ray.direction = Vector3d{0.0, 0.0, -1.0};
    std::optional<Intersection> isect;
    CHECK(bvh.intersect(ray, isect));
    CHECK(isect.has_value());
    CHECK(double(ray.tMax) == Catch::Approx(4.0).margin(1e-6));

    // Miss
    Ray miss_ray;
    miss_ray.origin = Point(5, 0, 0);
    miss_ray.direction = Vector3d{0.0, 1.0, 0.0};
    std::optional<Intersection> miss_isect;
    CHECK_FALSE(bvh.intersect(miss_ray, miss_isect));
}

TEST_CASE("bvh_transformed_sphere", "[accelerator][bvh]") {
    auto obj = std::make_shared<objects::Object>(
        *std::make_shared<geometry::Sphere>());
    obj->transform() =
        zipper::transform::Translation<double>(Vector3d{3.0, 0.0, 0.0})
            .to_transform();

    accel::BVHAccelerator bvh;
    bvh.build(*obj);

    Ray ray;
    ray.origin = Point(10, 0, 0);
    ray.direction = Vector3d{-1.0, 0.0, 0.0};
    std::optional<Intersection> isect;
    REQUIRE(bvh.intersect(ray, isect));
    REQUIRE(isect.has_value());

    // Hit at (4, 0, 0)
    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(4.0).margin(1e-6));
    CHECK(std::abs(pos(1)) < 1e-6);
    CHECK(std::abs(pos(2)) < 1e-6);
}

// ============================================================
// Nested scene graph (transforms compose)
// ============================================================

TEST_CASE("nested_transforms_compose", "[accelerator]") {
    // Inner group translates +x by 2, outer group translates +x by 3
    // Total: sphere should be at x=5
    auto sphere_obj = std::make_shared<objects::Object>(
        *std::make_shared<geometry::Sphere>());

    auto inner = objects::InternalSceneNode::create();
    inner->transform() =
        zipper::transform::Translation<double>(Vector3d{2.0, 0.0, 0.0})
            .to_transform();
    inner->add_node(sphere_obj);

    auto outer = objects::InternalSceneNode::create();
    outer->transform() =
        zipper::transform::Translation<double>(Vector3d{3.0, 0.0, 0.0})
            .to_transform();
    outer->add_node(inner);

    accel::LinearAccelerator accel;
    accel.build(*outer);

    // Should have 1 primitive with world transform = translate(5,0,0)
    REQUIRE(accel.primitives().size() == 1);

    Ray ray;
    ray.origin = Point(10, 0, 0);
    ray.direction = Vector3d{-1.0, 0.0, 0.0};
    std::optional<Intersection> isect;
    REQUIRE(accel.intersect(ray, isect));
    REQUIRE(isect.has_value());

    // Hit at x=6 (sphere surface at center 5, radius 1)
    Vector3d pos = Vector3d(isect->position);
    CHECK(pos(0) == Catch::Approx(6.0).margin(1e-6));
}

// ============================================================
// Empty scene
// ============================================================

TEST_CASE("empty_scene_returns_no_hit", "[accelerator]") {
    auto scene = objects::InternalSceneNode::create();

    accel::LinearAccelerator linear;
    linear.build(*scene);

    accel::BVHAccelerator bvh;
    bvh.build(*scene);

    Ray ray;
    ray.origin = Point(0, 0, 5);
    ray.direction = Vector3d{0.0, 0.0, -1.0};

    std::optional<Intersection> isect_lin;
    std::optional<Intersection> isect_bvh;
    CHECK_FALSE(linear.intersect(ray, isect_lin));
    CHECK_FALSE(bvh.intersect(ray, isect_bvh));
}
