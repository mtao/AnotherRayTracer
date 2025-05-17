
#pragma once

#include <zipper/Matrix.hpp>
#include <zipper/Vector.hpp>
#include <zipper/Form.hpp>

namespace art{
template <typename T, int R, int C>
using Matrix = zipper::Matrix<T, R, C>;

template <typename T, int R>
using Vector = zipper::Vector<T, R>;
template <typename T, int R>
using RowVector = zipper::Form<T, R>;

using Vector3d = Vector<double, 3>;
using Vector4d = Vector<double, 4>;
using Vector6d = Vector<double, 6>;
}  // namespace art::eigen
