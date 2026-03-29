#include "art/accel/SceneAccelerator.hpp"

#include "art/geometry/Geometry.hpp"
#include "art/objects/InternalSceneNode.hpp"
#include "art/objects/Object.hpp"

namespace art::accel {

auto SceneAccelerator::build(const objects::SceneNode &root) -> void {
    m_primitives.clear();
    // Identity transform at the root
    flatten(root, utils::AffineTransform{});
    build_index();
}

auto SceneAccelerator::flatten(const objects::SceneNode &node,
                               const utils::AffineTransform &parent_transform)
    -> void {
    // Accumulate world transform: parent * local
    auto world_transform = parent_transform * node.transform();

    // If this is a leaf Object, extract its geometry
    if (auto *obj = dynamic_cast<const objects::Object *>(&node)) {
        m_primitives.push_back(ScenePrimitive{
            .geometry = obj->geometry_ptr(),
            .world_transform = world_transform,
        });
        return;
    }

    // If this is an InternalSceneNode, recurse into children
    if (auto *group = dynamic_cast<const objects::InternalSceneNode *>(&node)) {
        for (const auto &child : group->children()) {
            flatten(*child, world_transform);
        }
    }
}

} // namespace art::accel
