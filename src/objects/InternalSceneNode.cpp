#include "art/objects/InternalSceneNode.hpp"

namespace art::objects {

auto InternalSceneNode::create() -> Ptr {
    return std::make_shared<InternalSceneNode>();
}

auto InternalSceneNode::add_node(SceneNode::Ptr node) -> void {
    _children.push_back(std::move(node));
}

} // namespace art::objects
