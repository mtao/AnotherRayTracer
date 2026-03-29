#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <memory>
#include <string>

#include "art/Camera.hpp"
#include "art/export.hpp"

namespace art {
class Image;
}

namespace art::objects {
class SceneNode;
}

namespace art::lua {

/// Parsed scene description from a Lua table DSL script.
struct ART_API SceneDescription {
    std::shared_ptr<objects::SceneNode> root;
    Camera camera{
        Camera::lookAt(Point(0, 0, 5), Point(0, 0, 0), Point(0, 1, 0))};
    size_t width = 800;
    size_t height = 600;
    std::string output_path = "output.ppm";
    std::string accelerator = "bvh";
};

/// Load a scene from a Lua script file.
/// The script must return a table with the scene description.
ART_API auto load_scene(const std::filesystem::path &script_path)
    -> std::expected<SceneDescription, std::string>;

/// Load a scene from a Lua source string.
ART_API auto load_scene_from_string(const std::string &lua_source)
    -> std::expected<SceneDescription, std::string>;

/// Render a loaded scene description and return the resulting image.
ART_API auto render_scene(const SceneDescription &desc)
    -> std::expected<Image, std::string>;

} // namespace art::lua
