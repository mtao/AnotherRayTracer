/// @file src/lua/bind_camera.cpp
/// Lua bindings for ART Camera.

#include "bind_internal.hpp"

#include "art/Camera.hpp"
#include "art/Point.hpp"

namespace art::lua {

void bind_camera(sol::table &tbl) {
    auto make_camera = [](const sol::table &pos,
                          const sol::table &target,
                          const sol::table &up) {
        return Camera(Camera::lookAt(Point(table_to_vec3(pos)),
                                     Point(table_to_vec3(target)),
                                     Point(table_to_vec3(up))));
    };

    tbl.new_usertype<Camera>("Camera",
                             sol::call_constructor,
                             sol::factories(make_camera),

                             "look_at",
                             make_camera);
}

} // namespace art::lua
