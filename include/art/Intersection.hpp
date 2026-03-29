#if !defined(ART_INTERSECTION_HPP)
#define ART_INTERSECTION_HPP
#include "art/types.hpp"

namespace art {
// An intersection is stored
struct Intersection {
    Rational t = 0;
    Point position;
    Vector3d normal;
    // std::shared_ptr<Material> material = nullptr;
};
}  // namespace art
#endif
