#pragma once
#include <cstdint>

#include "art/types.hpp"

namespace art {
namespace geometry {
    class Geometry;
}

// Intersection record following PBRT's two-phase pattern:
//   Phase 1 (geometric): populated by Geometry::intersect()
//   Phase 2 (shading):   populated later by the material system (future)
struct Intersection {
    // --- Phase 1: Geometric ---
    Rational t = 0;
    Point position;
    Vector3d geometric_normal; // face/surface normal (not necessarily unit)
    Vector2d uv{0.0, 0.0}; // texture coords or parametric coords
    Vector3d dpdu; // tangent vector: dp/du
    Vector3d dpdv; // tangent vector: dp/dv

    // Identification
    const geometry::Geometry *geometry = nullptr; // non-owning
    uint32_t primitive_index = 0;

    // --- Phase 2: Shading (future — populated by material system) ---
    // Vector3d shading_normal;
    // const Material* material = nullptr;
};
} // namespace art
