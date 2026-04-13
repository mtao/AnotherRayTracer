#pragma once

#include <zipper/transform/all.hpp>

#include "art/zipper_types.hpp"

namespace art::utils {

// Use zipper's transform types directly
using AffineTransform = zipper::transform::AffineTransform<double, 3>;
using Isometry = zipper::transform::Isometry<double, 3>;
using ProjectiveTransform = zipper::transform::ProjectiveTransform<double, 3>;

}  // namespace art::utils
