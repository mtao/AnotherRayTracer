#pragma once
#include <zipper/views/nullary/IdentityView.hpp>

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
}  // namespace art::utils
