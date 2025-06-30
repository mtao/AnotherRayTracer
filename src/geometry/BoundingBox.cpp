#include "art/geometry/BoundingBox.hpp"

#include <spdlog/spdlog.h>

#include <array>
namespace art::geometry {
BoundingBox& BoundingBox::expand(const BoundingBox& bb) {
    expand(bb._min);
    expand(bb._max);
    return *this;
}
BoundingBox& BoundingBox::expand(const Point& p) {
    spdlog::info("updating from point {} ", p);
    std::array<Rational, 3> nmin;
    std::array<Rational, 3> nmax;
    for (size_t i = 0; i < 3; ++i) {
        nmin[i] = ::art::min(p(i), this->_min(i));
        spdlog::info("update min {}:  {} {} [{}]=> {}", i, this->_min(i), p(i),
                     this->_min(i) < p(i), nmin[i]);
        nmax[i] = ::art::max(p(i), this->_max(i));
        spdlog::info("update max {}:  {} {} [{}]=> {}", i, this->_max(i), p(i),
                     this->_max(i) > p(i), nmax[i]);
    }
    _min = Point(nmin[0], nmin[1], nmin[2]);
    _max = Point(nmax[0], nmax[1], nmax[2]);
    spdlog::info("child {} {}", min(), max());
    return *this;
}

bool BoundingBox::contains(const Point& p) const {
    bool above_min = (p.denominator() * _min.numerator().as_array() <
                      _min.denominator() * p.numerator().as_array())
                         .all();
    bool below_max = (p.denominator() * _max.numerator().as_array() >
                      _max.denominator() * p.numerator().as_array())
                         .all();
    return above_min && below_max;
}
bool BoundingBox::contains(const BoundingBox& bb) const {
    return contains(bb.min()) && contains(bb.max());
}
}  // namespace art::geometry
