/// @file src/lua/bind_transform.cpp
/// Lua bindings for zipper transform types used by ART.

#include "bind_internal.hpp"

#include "art/utils/AffineTransform.hpp"

namespace art::lua {

void bind_transform(sol::table &tbl) {
    // ── AffineTransform ──
    tbl.new_usertype<utils::AffineTransform>(
        "AffineTransform",
        sol::call_constructor,
        sol::factories(
            // Default: identity
            [] { return utils::AffineTransform{}; }),

        "inverse",
        [](const utils::AffineTransform &xf) { return xf.inverse(); });

    // ── Transform factory functions ──
    tbl.set_function(
        "translation", [](const sol::table &t) -> utils::AffineTransform {
            auto v = table_to_vec3(t);
            return zipper::transform::Translation<double>(v).to_transform();
        });

    tbl.set_function(
        "rotation",
        [](double angle, const sol::table &axis) -> utils::AffineTransform {
            auto a = table_to_vec3(axis);
            return zipper::transform::AxisAngleRotation<double>(angle, a)
                .to_transform();
        });

    tbl.set_function(
        "scaling", [](const sol::object &arg) -> utils::AffineTransform {
            if (arg.is<sol::table>()) {
                auto v = table_to_vec3(arg.as<sol::table>());
                return zipper::transform::Scaling<double>(v).to_transform();
            }
            // Uniform scale
            double s = arg.as<double>();
            return zipper::transform::Scaling<double>(Vector3d{s, s, s})
                .to_transform();
        });

    // Compose: T * R * S (delegates to shared parse_transform)
    tbl.set_function("compose_trs", parse_transform);
}

} // namespace art::lua
