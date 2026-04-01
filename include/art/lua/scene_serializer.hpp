#pragma once
/// @file include/art/lua/scene_serializer.hpp
/// Serialize a SceneDescription back to Lua table DSL (round-trip).

#include <string>

#include "art/export.hpp"

namespace art::lua {

struct SceneDescription;

/// Serialize a scene to a Lua table string that can be loaded by
/// load_scene_from_string(). The output is a complete Lua script that
/// returns the scene table.
ART_API auto serialize_scene(const SceneDescription &desc) -> std::string;

} // namespace art::lua
