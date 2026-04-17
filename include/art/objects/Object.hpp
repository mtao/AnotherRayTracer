#if !defined(ART_OBJECTS_OBJECT_HPP)
#define ART_OBJECTS_OBJECT_HPP
#include <memory>

#include "art/export.hpp"
#include "art/objects/SceneNode.hpp"

namespace art::geometry {
class Geometry;
}
namespace art::objects {
class ART_API Object : public SceneNode {
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
#endif
