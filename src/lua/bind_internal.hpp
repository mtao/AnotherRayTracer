#pragma once
/// @file src/lua/bind_internal.hpp
/// Shared declarations for per-domain bind_*() functions.

#include <sol/sol.hpp>

#include <zipper/transform/transform.hpp>

#include "art/utils/AffineTransform.hpp"
#include "art/zipper_types.hpp"

namespace art::lua {

// Forward declarations for bind functions — called by bindings.cpp
// orchestrator.
void bind_geometry(sol::table &tbl);
void bind_scene(sol::table &tbl);
void bind_camera(sol::table &tbl);
void bind_image(sol::table &tbl);
void bind_transform(sol::table &tbl);

// ── Helper: read a Lua table {x, y, z} into a Vector3d ──

inline auto table_to_vec3(const sol::table &t) -> Vector3d {
    return Vector3d{
        t.get_or(1, 0.0),
        t.get_or(2, 0.0),
        t.get_or(3, 0.0),
    };
}

inline auto table_to_vec2(const sol::table &t) -> Vector2d {
    return Vector2d{
        t.get_or(1, 0.0),
        t.get_or(2, 0.0),
    };
}

// Push a Vector3d as a Lua table {x, y, z}
inline auto vec3_to_table(sol::state_view lua, const Vector3d &v)
    -> sol::table {
    sol::table t = lua.create_table(3, 0);
    t[1] = static_cast<double>(v(0));
    t[2] = static_cast<double>(v(1));
    t[3] = static_cast<double>(v(2));
    return t;
}

inline auto vec2_to_table(sol::state_view lua, const Vector2d &v)
    -> sol::table {
    sol::table t = lua.create_table(2, 0);
    t[1] = static_cast<double>(v(0));
    t[2] = static_cast<double>(v(1));
    return t;
}

// ── Helper: parse a TRS transform table ──
// Expected format: { translate = {x,y,z}, rotate = {angle=.., axis={..}},
//                    scale = number_or_vec3 }
// Composition order: T * R * S (scale applied first).

inline auto parse_transform(const sol::table &t) -> utils::AffineTransform {
    utils::AffineTransform result{};

    // Scale first (applied to geometry)
    if (auto scale = t.get<sol::optional<sol::object>>("scale");
        scale && scale->valid()) {
        if (scale->is<sol::table>()) {
            auto v = table_to_vec3(scale->as<sol::table>());
            result =
                zipper::transform::Scaling<double>(v).to_transform() * result;
        } else if (scale->is<double>()) {
            double s = scale->as<double>();
            result = zipper::transform::Scaling<double>(Vector3d{s, s, s})
                         .to_transform()
                     * result;
        }
    }

    // Rotate second
    if (auto rotate = t.get<sol::optional<sol::table>>("rotate");
        rotate && rotate->valid()) {
        double angle = rotate->get_or("angle", 0.0);
        sol::optional<sol::table> axis_t =
            rotate->get<sol::optional<sol::table>>("axis");
        if (axis_t) {
            auto axis = table_to_vec3(*axis_t);
            result = zipper::transform::AxisAngleRotation<double>(angle, axis)
                         .to_transform()
                     * result;
        }
    }

    // Translate last
    if (auto translate = t.get<sol::optional<sol::table>>("translate");
        translate && translate->valid()) {
        auto v = table_to_vec3(*translate);
        result =
            zipper::transform::Translation<double>(v).to_transform() * result;
    }

    return result;
}

} // namespace art::lua
