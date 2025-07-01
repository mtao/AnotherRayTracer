#pragma once
#include <memory>
#include <optional>

#include "art/Intersection.hpp"
#include "art/geometry/Box.hpp"

namespace art {
class Ray;
}
namespace art::objects {
class SceneNode {
   public:
    using Ptr = std::shared_ptr<SceneNode>;
    const geometry::Box& bounding_box() const { return _bounding_box; }
    virtual void update_bounding_box() = 0;
    void set_bounding_box(const geometry::Box& bounding_box) {
        _bounding_box = bounding_box;
    }

    bool intersects_bounding_box(const Ray& ray) const;

    virtual bool intersect(const Ray& ray,
                           std::optional<Intersection>& isect) const = 0;

   private:
    geometry::Box _bounding_box;
};
}  // namespace art::objects
