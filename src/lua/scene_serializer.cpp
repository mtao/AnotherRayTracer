/// @file src/lua/scene_serializer.cpp
/// Serialize a SceneDescription to Lua table DSL (round-trip).

#include "art/lua/scene_serializer.hpp"

#include <cmath>
#include <format>
#include <sstream>
#include <string>

#include "art/Camera.hpp"
#include "art/lua/scene_loader.hpp"
#include "art/objects/InternalSceneNode.hpp"
#include "art/objects/Object.hpp"
#include "art/utils/AffineTransform.hpp"

#include "art/geometry/Box.hpp"
#include "art/geometry/Cylinder.hpp"
#include "art/geometry/Disk.hpp"
#include "art/geometry/MeshGeometry.hpp"
#include "art/geometry/Plane.hpp"
#include "art/geometry/Sphere.hpp"
#include "art/geometry/Triangle.hpp"

#include <zipper/transform/decompose.hpp>

namespace art::lua {

namespace {

    // ── Formatting helpers ──

    auto fmt_double(double v) -> std::string {
        // Format with enough precision for round-trip but trim trailing zeros
        auto s = std::format("{:.10g}", v);
        return s;
    }

    auto fmt_vec3(const Vector3d &v) -> std::string {
        return std::format("{{{}, {}, {}}}",
                           fmt_double(v(0)),
                           fmt_double(v(1)),
                           fmt_double(v(2)));
    }

    auto indent(int depth) -> std::string {
        return std::string(depth * 4, ' ');
    }

    // ── Transform serialization ──

    auto is_identity(const utils::AffineTransform &xf) -> bool {
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                double expected = (r == c) ? 1.0 : 0.0;
                if (std::abs(static_cast<double>(xf(r, c)) - expected)
                    > 1e-12) {
                    return false;
                }
            }
        }
        return true;
    }

    auto serialize_transform(const utils::AffineTransform &xf, int depth)
        -> std::string {
        if (is_identity(xf)) { return ""; }

        auto [t, r, s] = zipper::transform::trs_decompose(xf);

        auto tv = t.vector();
        auto sf = s.factors();

        bool has_translate = std::abs(static_cast<double>(tv(0))) > 1e-12
                             || std::abs(static_cast<double>(tv(1))) > 1e-12
                             || std::abs(static_cast<double>(tv(2))) > 1e-12;

        bool is_uniform_scale =
            std::abs(static_cast<double>(sf(0)) - static_cast<double>(sf(1)))
                < 1e-12
            && std::abs(static_cast<double>(sf(1)) - static_cast<double>(sf(2)))
                   < 1e-12;
        bool has_scale = std::abs(static_cast<double>(sf(0)) - 1.0) > 1e-12
                         || std::abs(static_cast<double>(sf(1)) - 1.0) > 1e-12
                         || std::abs(static_cast<double>(sf(2)) - 1.0) > 1e-12;

        // Check if rotation is identity
        auto rm = r.matrix();
        bool has_rotation = false;
        for (int i = 0; i < 3 && !has_rotation; ++i) {
            for (int j = 0; j < 3 && !has_rotation; ++j) {
                double expected = (i == j) ? 1.0 : 0.0;
                if (std::abs(static_cast<double>(rm(i, j)) - expected)
                    > 1e-12) {
                    has_rotation = true;
                }
            }
        }

        if (!has_translate && !has_rotation && !has_scale) { return ""; }

        std::ostringstream out;
        out << indent(depth) << "transform = {\n";

        if (has_translate) {
            Vector3d tvec{
                static_cast<double>(tv(0)),
                static_cast<double>(tv(1)),
                static_cast<double>(tv(2)),
            };
            out << indent(depth + 1) << "translate = " << fmt_vec3(tvec)
                << ",\n";
        }

        if (has_rotation) {
            // Extract angle-axis from rotation matrix via Rodrigues' inverse
            // trace(R) = 1 + 2*cos(angle)
            double trace = static_cast<double>(rm(0, 0))
                           + static_cast<double>(rm(1, 1))
                           + static_cast<double>(rm(2, 2));
            double cos_angle = (trace - 1.0) / 2.0;
            cos_angle = std::clamp(cos_angle, -1.0, 1.0);
            double angle = std::acos(cos_angle);

            if (std::abs(angle) > 1e-12) {
                // axis = [R(2,1)-R(1,2), R(0,2)-R(2,0), R(1,0)-R(0,1)] /
                // (2*sin(angle))
                double sin_angle = std::sin(angle);
                Vector3d axis{
                    (static_cast<double>(rm(2, 1))
                     - static_cast<double>(rm(1, 2)))
                        / (2.0 * sin_angle),
                    (static_cast<double>(rm(0, 2))
                     - static_cast<double>(rm(2, 0)))
                        / (2.0 * sin_angle),
                    (static_cast<double>(rm(1, 0))
                     - static_cast<double>(rm(0, 1)))
                        / (2.0 * sin_angle),
                };
                out << indent(depth + 1)
                    << "rotate = {angle = " << fmt_double(angle)
                    << ", axis = " << fmt_vec3(axis) << "},\n";
            }
        }

        if (has_scale) {
            if (is_uniform_scale) {
                out << indent(depth + 1)
                    << "scale = " << fmt_double(static_cast<double>(sf(0)))
                    << ",\n";
            } else {
                Vector3d svec{
                    static_cast<double>(sf(0)),
                    static_cast<double>(sf(1)),
                    static_cast<double>(sf(2)),
                };
                out << indent(depth + 1) << "scale = " << fmt_vec3(svec)
                    << ",\n";
            }
        }

        out << indent(depth) << "},\n";
        return out.str();
    }

    // ── Geometry serialization ──

    auto serialize_geometry_fields(const geometry::Geometry &geom, int depth)
        -> std::string {
        std::ostringstream out;

        if (dynamic_cast<const geometry::Sphere *>(&geom)) {
            out << indent(depth) << "type = \"sphere\",\n";
        } else if (const auto *box =
                       dynamic_cast<const geometry::Box *>(&geom)) {
            out << indent(depth) << "type = \"box\",\n";
            Vector3d mn = box->min();
            Vector3d mx = box->max();
            // Only emit min/max if they differ from the default
            bool is_default = std::abs(mn(0) - (-0.5)) < 1e-12
                              && std::abs(mn(1) - (-0.5)) < 1e-12
                              && std::abs(mn(2) - (-0.5)) < 1e-12
                              && std::abs(mx(0) - 0.5) < 1e-12
                              && std::abs(mx(1) - 0.5) < 1e-12
                              && std::abs(mx(2) - 0.5) < 1e-12;
            if (!is_default) {
                out << indent(depth) << "min = " << fmt_vec3(mn) << ",\n";
                out << indent(depth) << "max = " << fmt_vec3(mx) << ",\n";
            }
        } else if (dynamic_cast<const geometry::Plane *>(&geom)) {
            out << indent(depth) << "type = \"plane\",\n";
        } else if (dynamic_cast<const geometry::Disk *>(&geom)) {
            out << indent(depth) << "type = \"disk\",\n";
        } else if (dynamic_cast<const geometry::Cylinder *>(&geom)) {
            out << indent(depth) << "type = \"cylinder\",\n";
        } else if (const auto *tri =
                       dynamic_cast<const geometry::Triangle *>(&geom)) {
            out << indent(depth) << "type = \"triangle\",\n";

            const auto &verts = tri->vertices();
            out << indent(depth) << "vertices = {" << fmt_vec3(verts[0]) << ", "
                << fmt_vec3(verts[1]) << ", " << fmt_vec3(verts[2]) << "},\n";

            if (tri->normals()) {
                const auto &norms = *tri->normals();
                out << indent(depth) << "normals = {" << fmt_vec3(norms[0])
                    << ", " << fmt_vec3(norms[1]) << ", " << fmt_vec3(norms[2])
                    << "},\n";
            }
        } else if (const auto *mesh =
                       dynamic_cast<const geometry::MeshGeometry *>(&geom)) {
            out << indent(depth) << "type = \"mesh\",\n";
            out << indent(depth) << "-- mesh with " << mesh->triangle_count()
                << " triangles (data omitted)\n";
        } else {
            out << indent(depth) << "type = \"unknown\",\n";
        }

        return out.str();
    }

    // ── Scene node serialization (recursive) ──

    auto serialize_node(const objects::SceneNode &node, int depth)
        -> std::string {
        std::ostringstream out;

        if (const auto *group =
                dynamic_cast<const objects::InternalSceneNode *>(&node)) {
            out << indent(depth) << "{\n";
            out << indent(depth + 1) << "type = \"group\",\n";
            out << serialize_transform(node.transform(), depth + 1);

            const auto &children = group->children();
            if (!children.empty()) {
                out << indent(depth + 1) << "children = {\n";
                for (const auto &child : children) {
                    if (child) { out << serialize_node(*child, depth + 2); }
                }
                out << indent(depth + 1) << "},\n";
            }
            out << indent(depth) << "},\n";
        } else if (const auto *obj =
                       dynamic_cast<const objects::Object *>(&node)) {
            out << indent(depth) << "{\n";
            out << serialize_geometry_fields(obj->geometry(), depth + 1);
            out << serialize_transform(node.transform(), depth + 1);
            out << indent(depth) << "},\n";
        }

        return out.str();
    }

    // ── Camera serialization ──

    auto serialize_camera(const Camera &cam, int depth) -> std::string {
        std::ostringstream out;

        // The camera stores an Isometry. We can extract eye position from
        // the inverse transform's translation (camera-to-world), and derive
        // target and up from the rotation columns.
        auto cam_to_world = cam.transform().inverse();
        auto tv = cam_to_world.translation();
        Vector3d eye{
            static_cast<double>(tv(0)),
            static_cast<double>(tv(1)),
            static_cast<double>(tv(2)),
        };

        // Camera looks along -Z in camera space, up is +Y
        auto lin = cam_to_world.linear();
        Vector3d forward{
            -static_cast<double>(lin(0, 2)),
            -static_cast<double>(lin(1, 2)),
            -static_cast<double>(lin(2, 2)),
        };
        Vector3d up{
            static_cast<double>(lin(0, 1)),
            static_cast<double>(lin(1, 1)),
            static_cast<double>(lin(2, 1)),
        };
        Vector3d target = (eye + forward).eval();

        out << indent(depth) << "camera = {\n";
        out << indent(depth + 1) << "position = " << fmt_vec3(eye) << ",\n";
        out << indent(depth + 1) << "target = " << fmt_vec3(target) << ",\n";
        out << indent(depth + 1) << "up = " << fmt_vec3(up) << ",\n";
        out << indent(depth) << "},\n";

        return out.str();
    }

} // anonymous namespace

// ── Public API ──

auto serialize_scene(const SceneDescription &desc) -> std::string {
    std::ostringstream out;
    out << "return {\n";

    // Render settings
    out << indent(1) << "render = {\n";
    out << indent(2) << "width = " << desc.width << ",\n";
    out << indent(2) << "height = " << desc.height << ",\n";
    out << indent(2) << "output = \"" << desc.output_path << "\",\n";
    out << indent(2) << "accelerator = \"" << desc.accelerator << "\",\n";
    out << indent(1) << "},\n";

    // Camera
    out << serialize_camera(desc.camera, 1);

    // Scene graph
    out << indent(1) << "scene = {\n";
    if (desc.root) {
        // If root is a group, emit its children directly at top level
        if (const auto *group =
                dynamic_cast<const objects::InternalSceneNode *>(
                    desc.root.get())) {
            // Only flatten if the group itself has identity transform
            if (is_identity(group->transform())) {
                for (const auto &child : group->children()) {
                    if (child) { out << serialize_node(*child, 2); }
                }
            } else {
                out << serialize_node(*desc.root, 2);
            }
        } else {
            out << serialize_node(*desc.root, 2);
        }
    }
    out << indent(1) << "},\n";

    out << "}\n";
    return out.str();
}

} // namespace art::lua
