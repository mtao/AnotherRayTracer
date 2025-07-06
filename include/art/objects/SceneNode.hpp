#pragma once
#include <memory>
#include <optional>

#include "art/Intersection.hpp"
#include "art/geometry/Box.hpp"
#include "art/utils/AffineTransform.hpp"

namespace art {
class Ray;
}
namespace art::objects {
class SceneNode {
   public:
    using Ptr = std::shared_ptr<SceneNode>;
    const geometry::Box& bounding_box() const { return _bounding_box; }
    virtual void update_bounding_box() = 0;

    bool intersects_bounding_box(const Ray& ray) const;

    // Transforms a ray to this node's local space and does a bounding box check
    // The bounding box is defined in the local space
    bool intersect(const Ray& ray, std::optional<Intersection>& isect) const;

    //
    virtual bool intersect_direct(const Ray& ray,
                                  std::optional<Intersection>& isect) const = 0;

    const utils::AffineTransform& transform() const { return _transform; }
    utils::AffineTransform& transform() { return _transform; }

   protected:
    void set_bounding_box(const geometry::Box& bounding_box);

   private:
    geometry::Box _bounding_box = geometry::Box(
        Point::infinity_position(), Point::negative_infinity_position());
    utils::AffineTransform _transform;
};
}  // namespace art::objects
