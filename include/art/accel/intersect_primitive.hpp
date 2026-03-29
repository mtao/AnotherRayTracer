#pragma once
#include <optional>

#include "art/Intersection.hpp"
#include "art/accel/ScenePrimitive.hpp"

namespace art {
class Ray;
}

namespace art::accel {

/// Transform a ray to a primitive's local space, intersect, and
/// transform the result back to world space.
///
/// This is the core intersection logic shared by all accelerators.
/// It replaces the old SceneNode::intersect() transform logic.
ART_API auto intersect_primitive(const ScenePrimitive &prim,
                                 const Ray &ray,
                                 std::optional<Intersection> &isect) -> bool;

} // namespace art::accel
