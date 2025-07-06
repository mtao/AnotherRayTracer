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
    void update_bounding_box() final override;
    bool intersect_direct(const Ray& ray, std::optional<Intersection>& isect)
        const final override;

    const geometry::Geometry& geometry() const { return *_geometry; }
    // static Ptr create();

   private:
    std::shared_ptr<const geometry::Geometry> _geometry;
    // AffineTransform _transform;
};
}  // namespace art::objects
