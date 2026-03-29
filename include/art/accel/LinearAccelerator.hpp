#pragma once
#include "art/accel/SceneAccelerator.hpp"

namespace art::accel {

/// Brute-force linear scan over all primitives.
///
/// Always correct — serves as a reference implementation and baseline
/// for testing BVH correctness.
class ART_API LinearAccelerator : public SceneAccelerator {
  public:
    auto intersect(const Ray &ray, std::optional<Intersection> &isect) const
        -> bool override;

  protected:
    auto build_index() -> void override;
};

} // namespace art::accel
