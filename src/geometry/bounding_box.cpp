#include "art/geometry/bounding_box.hpp"

#include <array>
namespace art::geometry {
BoundingBox& BoundingBox::expand(const BoundingBox& bb) {
    expand(bb.min);
    expand(bb.max);
    return *this;
}
BoundingBox& BoundingBox::expand(const Point& p) {
    std::array<Rational, 3> nmin;
    std::array<Rational, 3> nmax;
    for(size_t i = 0; i < 3; ++i) {
        nmin[i] = ::art::min(p(i),this->min(i));
        nmax[i] = ::art::max(p(i),this->max(i));
    }
    min = Point(nmin[0],nmin[1],nmin[2]);
    max = Point(nmax[0],nmax[1],nmax[2]);
    return *this;
}
}  // namespace art::geometry
