#include "art/geometry/Line.hpp"
#include <fmt/format.h>
#include <zipper/utils/maxCoeff.hpp>

namespace art {
Rational Line::distance(const Point& p) const {
    return (momentAsPoint() - p.cross(directionAsPoint())).norm();
}
Point Line::smallestPoint() const { return direction().cross(moment()); }

Rational Line::distance(const Line& l) const {
    // degenate case
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

    Line Line::commonPerpendicular(const Line& l) const {
        auto m1 = moment();
        auto m2 = l.moment();
        auto l1 = direction();
        auto l2 = l.direction();

        Line perp;
        auto m_perp = perp.moment();
        auto l_perp = perp.direction();

        auto cross = l1.cross(l2);

        // the original equation has this idvision
        //l_perp = cross;
        //m_perp = m1.cross(m2) - m2.cross(l1) + reciprocalProduct(l) * l1.dot(l2) * cross / cross.norm_powered<2>();
        // above devision should be equivalent to 
        l_perp = cross * cross.norm_powered<2>();
        m_perp = m1.cross(m2) - m2.cross(l1) + reciprocalProduct(l) * l1.dot(l2) * cross ;
        return perp;
    }
    Point Line::foot(const Line& l) const {

        auto m1 = moment();
        auto m2 = l.moment();
        auto l1 = direction();
        auto l2 = l.direction();

        auto cross = l1.cross(l2);

        return Point(m2.cross(l1.cross(cross)) - m1.dot(cross) * l2, cross.norm_powered<2>());

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
