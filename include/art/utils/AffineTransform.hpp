#pragma once
#include <zipper/views/nullary/IdentityView.hpp>

#include "art/Point.hpp"
#include "art/zipper_types.hpp"

namespace art::utils {

class AffineTransform : public Matrix4d {
   public:
    using Base = Matrix4d;
    using Base::Base;
    AffineTransform()
        : Base(zipper::views::nullary::IdentityView<double, 4, 4>()) {}

    AffineTransform(const Base& base) : Base(base) {}

    using Base::operator=;
    auto as_matrix() { return static_cast<Base&>(*this); }
    auto as_matrix() const { return static_cast<Base const&>(*this); }
    auto rotation() {
        return slice(zipper::static_slice<0, 3>(),
                     zipper::static_slice<0, 3>());
    }
    auto translation() {
        zipper::VectorBase t =
            slice(zipper::static_slice<0, 4>(), zipper::static_index_t<3>())
                .view();
        return t;
    }
    auto rotation() const {
        return slice(zipper::static_slice<0, 3>(),
                     zipper::static_slice<0, 3>());
    }
    auto translation() const {
        zipper::VectorBase t =
            slice(zipper::static_slice<0, 4>(), zipper::static_index_t<3>())
                .view();
        return t;
    }

    static AffineTransform translation(const Point& p);
    static AffineTransform rotation(Matrix3x3Like auto const& R);
    static AffineTransform axis_angle_rotation(Vector3Like auto const& axis,
                                               double angle);
    // AffineTransform& rotate(const Quaternion& axis_angle);
};

inline auto operator*(const AffineTransform& a,
                      zipper::concepts::VectorBaseDerived auto const& b) {
    return a.as_matrix() * b;
}
inline auto operator*(const AffineTransform& a,
                      zipper::concepts::MatrixBaseDerived auto const& b) {
    return a.as_matrix() * b;
}

inline AffineTransform operator*(const AffineTransform& a,
                                 const AffineTransform& b) {
    return a.as_matrix() * b.as_matrix();
}

inline AffineTransform AffineTransform::rotation(Matrix3x3Like auto const& r) {
    AffineTransform R;
    R.rotation() = r;
    return R;
}

AffineTransform AffineTransform::axis_angle_rotation(
    Vector3Like auto const& axis, double angle) {
    AffineTransform M;
    auto R = M.rotation();

    // borrowed from eigen angle axis
    double sin = std::sin(angle);
    double cos = std::cos(angle);

    // zipper::VectorBase axis =
    //     zipper::views::nullary::unit_vector<double, 3>(dimension);

    auto sin_axis = sin * axis;
    auto cos1_axis = (1.0 - cos) * axis;

    {
        double tmp = cos1_axis(0) * axis(1);
        R(0, 1) = tmp - sin_axis(2);
        R(1, 0) = tmp + sin_axis(2);
    }

    {
        double tmp = cos1_axis(0) * axis(2);
        R(0, 2) = tmp + sin_axis(1);
        R(2, 0) = tmp - sin_axis(1);
    }
    {
        double tmp = cos1_axis(1) * axis(2);
        R(1, 2) = tmp - sin_axis(0);
        R(2, 1) = tmp + sin_axis(0);
    }
    R.diagonal() = (cos1_axis.as_array() * axis.as_array() + cos).view();
    return M;
}
}  // namespace art::utils
