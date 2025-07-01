#pragma once
#include "art/objects/SceneNode.hpp"

namespace art::objects {
class InternalSceneNode : public SceneNode {
   public:
    using Ptr = std::shared_ptr<InternalSceneNode>;
    InternalSceneNode() {}
    void add_node(SceneNode::Ptr);
    void update_bounding_box() override;
    bool intersect(const Ray& ray,
                   std::optional<Intersection>& isect) const override;

    static Ptr create();

   private:
    std::vector<SceneNode::Ptr> _children;
    //AffineTransform _transform;
};
}  // namespace art::objects
