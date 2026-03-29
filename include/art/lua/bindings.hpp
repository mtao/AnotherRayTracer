#pragma once

#include <sol/forward.hpp>

namespace art::lua {

/// Register ART types into a sol::state or sol::state_view.
///
/// Binds geometry types (Sphere, Box, Plane, etc.), scene graph nodes
/// (Object, InternalSceneNode), Camera, Image, and transform utilities
/// into an "art" global Lua table.
///
/// Also loads quiver Lua bindings so scene scripts can use
/// quiver.read_mesh(), quiver.ObjReader, etc. for mesh I/O.
void load_bindings(sol::state_view lua);

} // namespace art::lua
