/// @file src/art_lua.cpp
/// CLI runner for ART Lua scene description files.
///
/// Usage: art-lua scene.lua [--output path] [--width N] [--height N]

#include <format>
#include <iostream>
#include <string>

#include <spdlog/spdlog.h>

#include "art/io/image_io.hpp"
#include "art/lua/scene_loader.hpp"

static void print_usage(const char *prog) {
    std::cerr << std::format(
        "Usage: {} <scene.lua> [options]\n"
        "\n"
        "Options:\n"
        "  --output <path>   Output image path (overrides scene)\n"
        "  --width <N>       Image width (overrides scene)\n"
        "  --height <N>      Image height (overrides scene)\n"
        "  --accel <type>    Accelerator: 'bvh' or 'linear'\n"
        "  --help            Show this help\n",
        prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string scene_path;
    std::string output_override;
    int width_override = -1;
    int height_override = -1;
    std::string accel_override;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        }
        if (arg == "--output" && i + 1 < argc) {
            output_override = argv[++i];
        } else if (arg == "--width" && i + 1 < argc) {
            width_override = std::stoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            height_override = std::stoi(argv[++i]);
        } else if (arg == "--accel" && i + 1 < argc) {
            accel_override = argv[++i];
        } else if (arg[0] != '-') {
            scene_path = arg;
        } else {
            std::cerr << std::format("Unknown option: {}\n", arg);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (scene_path.empty()) {
        std::cerr << "Error: no scene file specified\n";
        print_usage(argv[0]);
        return 1;
    }

    // Load scene
    auto desc_result = art::lua::load_scene(scene_path);
    if (!desc_result) {
        std::cerr << std::format("Error loading scene: {}\n",
                                 desc_result.error());
        return 1;
    }
    auto &desc = *desc_result;

    // Apply CLI overrides
    if (width_override > 0) desc.width = static_cast<size_t>(width_override);
    if (height_override > 0) desc.height = static_cast<size_t>(height_override);
    if (!output_override.empty()) desc.output_path = output_override;
    if (!accel_override.empty()) desc.accelerator = accel_override;

    spdlog::info("scene: {} -> {}x{} -> {}",
                 scene_path,
                 desc.width,
                 desc.height,
                 desc.output_path);

    // Render
    auto img_result = art::lua::render_scene(desc);
    if (!img_result) {
        std::cerr << std::format("Error rendering: {}\n", img_result.error());
        return 1;
    }

    // Save
    auto save_result = art::io::save(desc.output_path, *img_result);
    if (!save_result) {
        std::cerr << std::format("Error saving image: {}\n",
                                 std::string(save_result.error()));
        return 1;
    }

    spdlog::info("saved: {}", desc.output_path);
    return 0;
}
