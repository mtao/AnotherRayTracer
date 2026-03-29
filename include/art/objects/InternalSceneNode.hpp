#pragma once
#include <vector>

#include "art/export.hpp"
#include "art/objects/SceneNode.hpp"

namespace art::objects {
class ART_API InternalSceneNode : public SceneNode {
  public:
    using Ptr = std::shared_ptr<InternalSceneNode>;
    InternalSceneNode() {}

    auto add_node(SceneNode::Ptr node) -> void;

    auto children() const -> const std::vector<SceneNode::Ptr> & {
        return _children;
    }

    static auto create() -> Ptr;

  private:
    std::vector<SceneNode::Ptr> _children;
};
} // namespace art::objects
