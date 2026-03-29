#pragma once
#include <memory>

#include "art/export.hpp"
#include "art/utils/AffineTransform.hpp"

namespace art::geometry {
class Geometry;
}

namespace art::accel {

/// A flattened scene leaf: a geometry with its accumulated world transform.
///
/// Produced by walking the scene graph and composing transforms down to
/// each leaf Object.  The accelerator stores a flat array of these and
/// builds a spatial index over their world-space bounding volumes.
struct ART_API ScenePrimitive {
    std::shared_ptr<const geometry::Geometry> geometry;
    utils::AffineTransform world_transform;
};

} // namespace art::accel
