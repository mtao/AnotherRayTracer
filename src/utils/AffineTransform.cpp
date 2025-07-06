
#include "art/utils/AffineTransform.hpp"

#include <zipper/views/nullary/UnitView.hpp>
namespace art::utils {
AffineTransform AffineTransform::translation(const Point& p) {
    AffineTransform T;
    T.translation() = p.homogeneous();
    return T;
}
/*
AffineTransform AffineTransform::axis_angle_rotation(
    zipper::rank_type dimension, double angle) {
    AffineTransform M;
    auto R = M.rotation();

    // borrowed from eigen angle axis
    double sin = std::sin(angle);
    double cos = std::cos(angle);

    zipper::VectorBase axis =
        zipper::views::nullary::unit_vector<double, 3>(dimension);

    auto sin_axis = sin * axis;
    auto cos1_axis = (1.0 - cos) * axis;

    double tmp = cos1_axis.x() * return {};
}
*/
}  // namespace art::utils
