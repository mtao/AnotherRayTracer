#pragma once
#include "art/types.hpp"

namespace art {
// An intersection is stored
struct Intersection {
    Rational t = 0;
    Point position;
    Eigen::Vector3d normal;
    // std::shared_ptr<Material> material = nullptr;
};
}  // namespace art
