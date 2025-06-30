#pragma once
#include "art/Point.hpp"

namespace art::geometry {
class BoundingBox {
   public:
    BoundingBox(const Point& min, const Point& max) : _min(min), _max(max) {}
    BoundingBox() : _min(), _max() {}
    BoundingBox(const BoundingBox& other) = default;
    BoundingBox(BoundingBox&& other) = default;
    BoundingBox& operator=(const BoundingBox& other) = default;
    BoundingBox& operator=(BoundingBox&& other) = default;

    BoundingBox& expand(const BoundingBox& bb);
    BoundingBox& expand(const Point& p);
    bool contains(const Point& p) const;
    bool contains(const BoundingBox& bb) const;
    const Point& min() const { return _min; }
    const Point& max() const { return _max; }

   private:
    Point _min;
    Point _max;
};
}  // namespace art::geometry
