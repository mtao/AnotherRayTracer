#pragma once
#include <optional>
#include <vector>

#include "art/accel/ScenePrimitive.hpp"
#include "art/export.hpp"

namespace art {
class Ray;
struct Intersection;
} // namespace art

namespace art::objects {
class SceneNode;
}

namespace art::accel {

/// Abstract base for scene-level ray intersection acceleration.
///
/// Decouples the spatial acceleration strategy from the scene graph.
/// The scene graph defines logical structure (transforms, grouping);
/// the accelerator provides efficient ray queries over a flattened
/// list of primitives with world transforms.
class ART_API SceneAccelerator {
  public:
    virtual ~SceneAccelerator() = default;

    /// Build the accelerator from a scene graph root.
    /// Walks the tree, flattens leaf Objects into ScenePrimitives with
    /// accumulated world transforms, and builds the internal spatial index.
    auto build(const objects::SceneNode &root) -> void;

    /// Cast a ray and find the closest intersection.
    /// Returns true if any primitive was hit.
    virtual auto intersect(const Ray &ray,
                           std::optional<Intersection> &isect) const
        -> bool = 0;

    /// Access the flattened primitive list (populated by build()).
    auto primitives() const -> const std::vector<ScenePrimitive> & {
        return m_primitives;
    }

  protected:
    std::vector<ScenePrimitive> m_primitives;

    /// Called after m_primitives is populated.  Subclasses build their
    /// spatial index here.
    virtual auto build_index() -> void = 0;

  private:
    /// Recursive helper: walks the scene graph accumulating transforms.
    auto flatten(const objects::SceneNode &node,
                 const utils::AffineTransform &parent_transform) -> void;
};

} // namespace art::accel
