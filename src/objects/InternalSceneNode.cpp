#include "art/objects/InternalSceneNode.hpp"

#include <spdlog/spdlog.h>

namespace art::objects {

auto InternalSceneNode::create() -> Ptr {
    return std::make_shared<InternalSceneNode>();
}
void InternalSceneNode::update_bounding_box() {
    geometry::Box bounding_box;
    for (auto&& c : _children) {
        c->update_bounding_box();
        // TODO: update coordinate system
        bounding_box.expand(c->bounding_box());
    }
    spdlog::info("{}", bounding_box);
    set_bounding_box(bounding_box);
}
void InternalSceneNode::add_node(SceneNode::Ptr node) {
    _children.push_back(node);
    set_bounding_box(
        geometry::Box(bounding_box()).expand(node->bounding_box()));
}
bool InternalSceneNode::intersect(const Ray& ray,
                                  std::optional<Intersection>& isect) const {
    if (intersects_bounding_box(ray)) {
        bool succ = false;
        // TODO: update coordinate system
        for (auto&& c : _children) {
            succ |= c->intersect(ray, isect);
        }
        return succ;
    }
    return false;
}
}  // namespace art::objects
