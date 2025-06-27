#pragma once
#include <memory>
#include <optional>

#include "art/geometry/BoundingBox.hpp"
#include "art/geometry/Ray.hpp"
#include "art/Intersection.hpp"

namespace art::objects {
class Object {
   public:
    using Ptr = std::shared_ptr<Object>;
    const geometry::BoundingBox& bbox() const { return _bbox; }
    virtual void update_bbox() = 0;
    void set_bbox(const geometry::BoundingBox& bbox) { _bbox = bbox; }

    virtual bool intersect(const geometry::Ray& ray,
                           std::optional<Intersection>& isect) const = 0;
    bool intersect(const geometry::Ray& ray, Intersection& isect) const;

   private:
    geometry::BoundingBox _bbox;
};
}  // namespace art::objects
