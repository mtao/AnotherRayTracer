/// @file src/lua/bind_scene.cpp
/// Lua bindings for ART scene graph nodes.

#include "bind_internal.hpp"

#include "art/geometry/Geometry.hpp"
#include "art/objects/InternalSceneNode.hpp"
#include "art/objects/Object.hpp"
#include "art/objects/SceneNode.hpp"

namespace art::lua {

void bind_scene(sol::table &tbl) {
    // ── SceneNode (base) ──
    tbl.new_usertype<objects::SceneNode>(
        "SceneNode",
        sol::no_constructor,

        "transform",
        sol::property(
            [](const objects::SceneNode &n) -> const utils::AffineTransform & {
                return n.transform();
            },
            [](objects::SceneNode &n, const utils::AffineTransform &xf) {
                n.transform() = xf;
            }));

    // ── Object (leaf node with geometry) ──
    tbl.new_usertype<objects::Object>(
        "Object",
        sol::call_constructor,
        sol::factories([](std::shared_ptr<geometry::Geometry> geom) {
            return std::make_shared<objects::Object>(*geom);
        }),
        sol::base_classes,
        sol::bases<objects::SceneNode>(),

        "geometry",
        [](const objects::Object &o) { return o.geometry_ptr(); });

    // ── InternalSceneNode (group node) ──
    tbl.new_usertype<objects::InternalSceneNode>(
        "Group",
        sol::call_constructor,
        sol::factories([] { return objects::InternalSceneNode::create(); }),
        sol::base_classes,
        sol::bases<objects::SceneNode>(),

        "add",
        [](objects::InternalSceneNode &g, objects::SceneNode::Ptr node) {
            g.add_node(std::move(node));
        },
        "children",
        [](const objects::InternalSceneNode &g) { return g.children(); });
}

} // namespace art::lua
