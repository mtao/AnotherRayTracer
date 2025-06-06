#include "art/line.hpp"
#include <fmt/format.h>
#include <zipper/utils/maxCoeff.hpp>

namespace art {
Rational Line::distance(const Point& p) const {
    return (momentAsPoint() - p.cross(directionAsPoint())).norm();
}
Point Line::smallestPoint() const { return direction().cross(moment()); }

Rational Line::distance(const Line& l) const {
    // deternate case
    if (direction().cross(direction()).norm_powered<2>() < 1e-8 * direction().norm()) {
        auto d = direction();
        auto ld = l.direction();
        int idx = std::get<1>(zipper::utils::maxCoeffWithIndex(d.as_array().abs()))[0];
        Rational s{ld(idx), d(idx)};
        return directionAsPoint()
                   .cross(momentAsPoint() - l.momentAsPoint() / s)
                   .norm() /
               directionAsPoint().squaredNorm();
    } else {
        return Rational(std::abs(reciprocalProduct(l))) / direction().norm_powered<2>();
    }
}
double Line::reciprocalProduct(const Line& l) const {
    return moment().dot(l.direction()) + direction().dot(l.moment());
}


Line::operator std::string() const {
   return fmt::format("Line(mom={},{},{};dir={},{},{})",
           moment()(0),
           moment()(1),
           moment()(2),
           direction()(0),
           direction()(1),
           direction()(2));
}
}  // namespace art
