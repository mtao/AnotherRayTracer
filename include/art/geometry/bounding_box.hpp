#pragma once
#include "art/point.hpp"

namespace art::geometry {
class BoundingBox {
   public:
    BoundingBox(const Point& min, const Point& max) : min(min), max(max) {}
    BoundingBox() : min(), max() {}
    BoundingBox(const BoundingBox& other) = default;
    BoundingBox(BoundingBox&& other) = default;
    BoundingBox& operator=(const BoundingBox& other) = default;
    BoundingBox& operator=(BoundingBox&& other) = default;
    Point min;
    Point max;

    BoundingBox& expand(const BoundingBox& bb);
    BoundingBox& expand(const Point& p);
};
}  // namespace art::geometry
