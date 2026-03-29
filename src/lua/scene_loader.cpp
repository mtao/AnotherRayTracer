/// @file src/lua/scene_loader.cpp
/// Lua table DSL scene loader: parses a Lua table into a SceneDescription.

#include "art/lua/scene_loader.hpp"

#include "bind_internal.hpp"

#include <filesystem>
#include <format>

#include <spdlog/spdlog.h>

#include "art/Camera.hpp"
#include "art/Image.hpp"
#include "art/Point.hpp"
#include "art/accel/BVHAccelerator.hpp"
#include "art/accel/LinearAccelerator.hpp"
#include "art/geometry/Box.hpp"
#include "art/geometry/Cylinder.hpp"
#include "art/geometry/Disk.hpp"
#include "art/geometry/MeshGeometry.hpp"
#include "art/geometry/Plane.hpp"
#include "art/geometry/Sphere.hpp"
#include "art/geometry/Triangle.hpp"
#include "art/geometry/mesh_utils.hpp"
#include "art/io/image_io.hpp"
#include "art/lua/bindings.hpp"
#include "art/objects/InternalSceneNode.hpp"
#include "art/objects/Object.hpp"

namespace art::lua {

namespace {

    // ── Geometry construction from table ──

    auto make_geometry(const sol::table &node)
        -> std::shared_ptr<const geometry::Geometry> {
        std::string type = node.get_or<std::string>("type", "");

        if (type == "sphere") { return std::make_shared<geometry::Sphere>(); }
        if (type == "box") {
            sol::optional<sol::table> min_t =
                node.get<sol::optional<sol::table>>("min");
            sol::optional<sol::table> max_t =
                node.get<sol::optional<sol::table>>("max");
            if (min_t && max_t) {
                auto mn = table_to_vec3(*min_t);
                auto mx = table_to_vec3(*max_t);
                return std::make_shared<geometry::Box>(Point(mn), Point(mx));
            }
            return std::make_shared<geometry::Box>();
        }
        if (type == "plane") { return std::make_shared<geometry::Plane>(); }
        if (type == "disk") { return std::make_shared<geometry::Disk>(); }
        if (type == "cylinder") {
            return std::make_shared<geometry::Cylinder>();
        }
        if (type == "triangle") {
            sol::optional<sol::table> verts =
                node.get<sol::optional<sol::table>>("vertices");
            if (!verts || verts->size() < 3) {
                spdlog::warn("triangle node missing 'vertices' (need 3 vec3)");
                return nullptr;
            }
            sol::table v0t = (*verts)[1];
            sol::table v1t = (*verts)[2];
            sol::table v2t = (*verts)[3];
            auto v0 = table_to_vec3(v0t);
            auto v1 = table_to_vec3(v1t);
            auto v2 = table_to_vec3(v2t);

            // Optional normals
            sol::optional<sol::table> normals =
                node.get<sol::optional<sol::table>>("normals");
            if (normals && normals->size() >= 3) {
                sol::table n0t = (*normals)[1];
                sol::table n1t = (*normals)[2];
                sol::table n2t = (*normals)[3];
                return std::make_shared<geometry::Triangle>(v0,
                                                            v1,
                                                            v2,
                                                            table_to_vec3(n0t),
                                                            table_to_vec3(n1t),
                                                            table_to_vec3(n2t));
            }
            return std::make_shared<geometry::Triangle>(v0, v1, v2);
        }
        if (type == "mesh") {
            sol::optional<sol::table> verts_t =
                node.get<sol::optional<sol::table>>("vertices");
            sol::optional<sol::table> tris_t =
                node.get<sol::optional<sol::table>>("triangles");
            if (!verts_t || !tris_t) {
                spdlog::warn("mesh node missing 'vertices' or 'triangles'");
                return nullptr;
            }

            using Vec3 = std::array<double, 3>;
            using Tri = std::array<int64_t, 3>;

            std::vector<Vec3> vertices;
            vertices.reserve(verts_t->size());
            for (size_t i = 1; i <= verts_t->size(); ++i) {
                sol::table v = (*verts_t)[i];
                vertices.push_back(
                    {v.get_or(1, 0.0), v.get_or(2, 0.0), v.get_or(3, 0.0)});
            }

            std::vector<Tri> triangles;
            triangles.reserve(tris_t->size());
            for (size_t i = 1; i <= tris_t->size(); ++i) {
                sol::table t = (*tris_t)[i];
                triangles.push_back({t.get_or<int64_t>(1, 0),
                                     t.get_or<int64_t>(2, 0),
                                     t.get_or<int64_t>(3, 0)});
            }

            return geometry::make_mesh_geometry(vertices, triangles);
        }
        if (type == "cube_mesh") { return geometry::make_cube_mesh(); }

        return nullptr;
    }

    // ── Scene node construction (recursive) ──

    auto parse_node(const sol::table &node) -> objects::SceneNode::Ptr {
        std::string type = node.get_or<std::string>("type", "");

        // Parse transform if present
        utils::AffineTransform xf{};
        sol::optional<sol::table> xf_t =
            node.get<sol::optional<sol::table>>("transform");
        if (xf_t) { xf = parse_transform(*xf_t); }

        if (type == "group") {
            auto group = objects::InternalSceneNode::create();
            group->transform() = xf;

            sol::optional<sol::table> children =
                node.get<sol::optional<sol::table>>("children");
            if (children) {
                for (size_t i = 1; i <= children->size(); ++i) {
                    sol::table child = (*children)[i];
                    auto child_node = parse_node(child);
                    if (child_node) { group->add_node(child_node); }
                }
            }
            return group;
        }

        // Leaf node: geometry object
        auto geom = make_geometry(node);
        if (!geom) {
            spdlog::warn("unknown or invalid geometry type: '{}'", type);
            return nullptr;
        }

        auto obj = std::make_shared<objects::Object>(*geom);
        obj->transform() = xf;
        return obj;
    }

    // ── Camera parsing ──

    auto parse_camera(const sol::table &t) -> Camera {
        sol::optional<sol::table> pos_t =
            t.get<sol::optional<sol::table>>("position");
        sol::optional<sol::table> target_t =
            t.get<sol::optional<sol::table>>("target");
        sol::optional<sol::table> up_t = t.get<sol::optional<sol::table>>("up");

        Vector3d pos = pos_t ? table_to_vec3(*pos_t) : Vector3d{0.0, 0.0, 5.0};
        Vector3d target =
            target_t ? table_to_vec3(*target_t) : Vector3d{0.0, 0.0, 0.0};
        Vector3d up = up_t ? table_to_vec3(*up_t) : Vector3d{0.0, 1.0, 0.0};

        return Camera(Camera::lookAt(Point(pos), Point(target), Point(up)));
    }

    // ── Core loader: table → SceneDescription ──

    auto parse_scene_table(const sol::table &tbl)
        -> std::expected<SceneDescription, std::string> {
        SceneDescription desc;

        // Parse render settings
        sol::optional<sol::table> render =
            tbl.get<sol::optional<sol::table>>("render");
        if (render) {
            desc.width = render->get_or<size_t>("width", 800);
            desc.height = render->get_or<size_t>("height", 600);
            desc.output_path =
                render->get_or<std::string>("output", "output.ppm");
            desc.accelerator =
                render->get_or<std::string>("accelerator", "bvh");
        }

        // Parse camera
        sol::optional<sol::table> camera =
            tbl.get<sol::optional<sol::table>>("camera");
        if (camera) { desc.camera = parse_camera(*camera); }

        // Parse scene graph
        sol::optional<sol::table> scene =
            tbl.get<sol::optional<sol::table>>("scene");
        if (!scene) {
            return std::unexpected("scene table missing 'scene' key");
        }

        // If there's only one top-level node, use it directly.
        // Otherwise, wrap in a group.
        size_t count = scene->size();
        if (count == 0) {
            return std::unexpected("scene table 'scene' is empty");
        }
        if (count == 1) {
            sol::table node = (*scene)[1];
            desc.root = parse_node(node);
        } else {
            auto group = objects::InternalSceneNode::create();
            for (size_t i = 1; i <= count; ++i) {
                sol::table node = (*scene)[i];
                auto child = parse_node(node);
                if (child) { group->add_node(child); }
            }
            desc.root = group;
        }

        if (!desc.root) {
            return std::unexpected("failed to construct any scene nodes");
        }

        return desc;
    }

} // anonymous namespace

// ── Public API ──

auto load_scene(const std::filesystem::path &script_path)
    -> std::expected<SceneDescription, std::string> {
    if (!std::filesystem::exists(script_path)) {
        return std::unexpected(
            std::format("scene file not found: {}", script_path.string()));
    }

    sol::state lua;
    lua.open_libraries(sol::lib::base,
                       sol::lib::string,
                       sol::lib::math,
                       sol::lib::table,
                       sol::lib::io,
                       sol::lib::os);

    load_bindings(lua);

    // Set the script's directory as a search path so require() works
    auto parent = script_path.parent_path();
    if (!parent.empty()) {
        lua["package"]["path"] =
            parent.string() + "/?.lua;"
            + lua["package"]["path"].get_or<std::string>("");
    }

    auto result =
        lua.safe_script_file(script_path.string(), sol::script_pass_on_error);
    if (!result.valid()) {
        sol::error err = result;
        return std::unexpected(std::format("Lua error: {}", err.what()));
    }

    // The script must return a table
    sol::object retval = result;
    if (retval.get_type() != sol::type::table) {
        return std::unexpected("scene script must return a table");
    }

    return parse_scene_table(retval.as<sol::table>());
}

auto load_scene_from_string(const std::string &lua_source)
    -> std::expected<SceneDescription, std::string> {
    sol::state lua;
    lua.open_libraries(
        sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table);

    load_bindings(lua);

    auto result = lua.safe_script(lua_source, sol::script_pass_on_error);
    if (!result.valid()) {
        sol::error err = result;
        return std::unexpected(std::format("Lua error: {}", err.what()));
    }

    sol::object retval = result;
    if (retval.get_type() != sol::type::table) {
        return std::unexpected("scene script must return a table");
    }

    return parse_scene_table(retval.as<sol::table>());
}

auto render_scene(const SceneDescription &desc)
    -> std::expected<Image, std::string> {
    if (!desc.root) { return std::unexpected("scene has no root node"); }

    // Build accelerator
    std::unique_ptr<accel::SceneAccelerator> accel;
    if (desc.accelerator == "linear") {
        accel = std::make_unique<accel::LinearAccelerator>();
    } else {
        accel = std::make_unique<accel::BVHAccelerator>();
    }
    accel->build(*desc.root);

    spdlog::info("rendering {}x{} with {} accelerator ({} primitives)",
                 desc.width,
                 desc.height,
                 desc.accelerator,
                 accel->primitives().size());

    Image img = desc.camera.render(desc.width, desc.height, *accel);
    return img;
}

} // namespace art::lua
