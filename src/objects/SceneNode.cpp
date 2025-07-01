#include "art/objects/SceneNode.hpp"

#include "art/geometry/Geometry.hpp"
namespace art::objects {

bool SceneNode::intersects_bounding_box(const Ray& ray) const {
    return bounding_box().intersect(ray);
}

// static Ptr create();

}  // namespace art::objects
