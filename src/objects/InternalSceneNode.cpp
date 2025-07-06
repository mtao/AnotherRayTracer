#include "art/objects/InternalSceneNode.hpp"


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
    set_bounding_box(bounding_box);
}
void InternalSceneNode::add_node(SceneNode::Ptr node) {
    _children.push_back(node);
    set_bounding_box(
        geometry::Box(bounding_box()).expand(node->bounding_box()));
}
bool InternalSceneNode::intersect_direct(
    const Ray& ray, std::optional<Intersection>& isect) const {
    bool succ = false;
    // TODO: update coordinate system
    for (auto&& c : _children) {
        succ |= c->intersect(ray, isect);
    }
    return succ;
}
}  // namespace art::objects
