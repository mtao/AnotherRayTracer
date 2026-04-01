/// @file src/lua/bindings.cpp
/// Orchestrator: registers all ART types into the "art" Lua table.
///
/// Also loads quiver's Lua bindings so scene scripts can use
/// quiver.read_mesh(), quiver.ObjReader, etc.

#include "art/lua/bindings.hpp"

#include <quiver/lua/bindings.hpp>

#include "bind_internal.hpp"

namespace art::lua {

void load_bindings(sol::state_view lua) {
    sol::table tbl = lua["art"].get_or_create<sol::table>();

    // Guard against double-registration (same pattern as quiver).
    if (tbl["Sphere"].valid()) return;

    bind_transform(tbl);
    bind_geometry(tbl);
    bind_scene(tbl);
    bind_camera(tbl);
    bind_image(tbl);

    // Load quiver bindings so scene scripts can call quiver.read_mesh() etc.
    quiver::lua::load_bindings(lua);
}

} // namespace art::lua
