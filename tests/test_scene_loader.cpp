/// @file tests/test_scene_loader.cpp
/// Tests for the Lua table DSL scene loader.

#include <catch2/catch_all.hpp>

#include "art/lua/scene_loader.hpp"
#include "art/objects/InternalSceneNode.hpp"
#include "art/objects/Object.hpp"

using namespace art;

TEST_CASE("load_scene_from_string: single sphere", "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            render = {
                width = 100,
                height = 100,
                output = "test.ppm",
            },
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                { type = "sphere" },
            },
        }
    )lua");

    REQUIRE(result.has_value());
    auto &desc = *result;
    CHECK(desc.width == 100);
    CHECK(desc.height == 100);
    CHECK(desc.output_path == "test.ppm");
    REQUIRE(desc.root != nullptr);
}

TEST_CASE("load_scene_from_string: multiple objects", "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                { type = "sphere" },
                { type = "box" },
                { type = "plane" },
            },
        }
    )lua");

    REQUIRE(result.has_value());
    auto &desc = *result;

    // Multiple top-level objects get wrapped in a group
    auto group =
        std::dynamic_pointer_cast<objects::InternalSceneNode>(desc.root);
    REQUIRE(group != nullptr);
    CHECK(group->children().size() == 3);
}

TEST_CASE("load_scene_from_string: transform parsing", "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                {
                    type = "sphere",
                    transform = {
                        translate = {1, 2, 3},
                    },
                },
            },
        }
    )lua");

    REQUIRE(result.has_value());
    REQUIRE(result->root != nullptr);
}

TEST_CASE("load_scene_from_string: group with children",
          "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                {
                    type = "group",
                    transform = { translate = {0, 1, 0} },
                    children = {
                        { type = "sphere" },
                        { type = "box" },
                    },
                },
            },
        }
    )lua");

    REQUIRE(result.has_value());
    auto group =
        std::dynamic_pointer_cast<objects::InternalSceneNode>(result->root);
    REQUIRE(group != nullptr);
    CHECK(group->children().size() == 2);
}

TEST_CASE("load_scene_from_string: all geometry types", "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                { type = "sphere" },
                { type = "box" },
                { type = "box", min = {-1, -1, -1}, max = {1, 1, 1} },
                { type = "plane" },
                { type = "disk" },
                { type = "cylinder" },
                { type = "triangle", vertices = {{0,0,0}, {1,0,0}, {0,1,0}} },
                { type = "cube_mesh" },
            },
        }
    )lua");

    REQUIRE(result.has_value());
    auto group =
        std::dynamic_pointer_cast<objects::InternalSceneNode>(result->root);
    REQUIRE(group != nullptr);
    CHECK(group->children().size() == 8);
}

TEST_CASE("load_scene_from_string: inline mesh", "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                {
                    type = "mesh",
                    vertices = {
                        {0, 0, 0},
                        {1, 0, 0},
                        {0, 1, 0},
                        {1, 1, 0},
                    },
                    triangles = {
                        {0, 1, 2},
                        {1, 3, 2},
                    },
                },
            },
        }
    )lua");

    REQUIRE(result.has_value());
    REQUIRE(result->root != nullptr);
}

TEST_CASE("load_scene_from_string: render defaults", "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                { type = "sphere" },
            },
        }
    )lua");

    REQUIRE(result.has_value());
    CHECK(result->width == 800);
    CHECK(result->height == 600);
    CHECK(result->output_path == "output.ppm");
    CHECK(result->accelerator == "bvh");
}

TEST_CASE("load_scene_from_string: missing scene key", "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
        }
    )lua");

    REQUIRE(!result.has_value());
    CHECK(result.error().find("scene") != std::string::npos);
}

TEST_CASE("load_scene_from_string: Lua syntax error", "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string("this is not valid lua }{}{");

    REQUIRE(!result.has_value());
    CHECK(result.error().find("Lua error") != std::string::npos);
}

TEST_CASE("load_scene_from_string: script returns non-table",
          "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return 42
    )lua");

    REQUIRE(!result.has_value());
    CHECK(result.error().find("table") != std::string::npos);
}

TEST_CASE("load_scene_from_string: accelerator selection",
          "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            render = { accelerator = "linear" },
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                { type = "sphere" },
            },
        }
    )lua");

    REQUIRE(result.has_value());
    CHECK(result->accelerator == "linear");
}

TEST_CASE("render_scene: basic render produces image", "[lua][scene_loader]") {
    auto desc_result = lua::load_scene_from_string(R"lua(
        return {
            render = {
                width = 10,
                height = 10,
            },
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                { type = "sphere" },
            },
        }
    )lua");

    REQUIRE(desc_result.has_value());

    auto img_result = lua::render_scene(*desc_result);
    REQUIRE(img_result.has_value());
    CHECK(img_result->width() == 10);
    CHECK(img_result->height() == 10);
}

TEST_CASE("load_scene_from_string: transform with scale + rotate + translate",
          "[lua][scene_loader]") {
    auto result = lua::load_scene_from_string(R"lua(
        return {
            camera = {
                position = {0, 0, 5},
                target = {0, 0, 0},
                up = {0, 1, 0},
            },
            scene = {
                {
                    type = "sphere",
                    transform = {
                        translate = {1, 0, 0},
                        rotate = {angle = 1.57, axis = {0, 1, 0}},
                        scale = {2, 2, 2},
                    },
                },
            },
        }
    )lua");

    REQUIRE(result.has_value());
    REQUIRE(result->root != nullptr);
}
