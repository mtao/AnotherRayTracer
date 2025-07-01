#pragma once
#include <memory>

#include "art/objects/SceneNode.hpp"

namespace art::geometry {
class Geometry;
}
namespace art::objects {
class Object : public SceneNode {
   public:
    using Ptr = std::shared_ptr<Object>;
    Object(const geometry::Geometry& geometry);
    ~Object();
    void update_bounding_box() override;
    bool intersect(const Ray& ray,
                   std::optional<Intersection>& isect) const override;

    // static Ptr create();

   private:
    std::shared_ptr<const geometry::Geometry> _geometry;
    // AffineTransform _transform;
};
}  // namespace art::objects
