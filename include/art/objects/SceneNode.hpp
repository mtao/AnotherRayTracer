#pragma once
#include "art/objects/Object.hpp"

namespace art::objects {
class SceneNode : public Object {
   public:
    using Ptr = std::shared_ptr<SceneNode>;
    SceneNode() {}
    void add_node(Object::Ptr);
    void update_bbox() override;
    bool intersect(const geometry::Ray& ray,
                   std::optional<Intersection>& isect) const override;

    static Ptr create();

   private:
    std::vector<Object::Ptr> _children;
    //AffineTransform _transform;
};
}  // namespace art::objects
