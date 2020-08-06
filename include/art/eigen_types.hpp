#pragma once

#include <Eigen/Dense>

namespace art::eigen {
template <typename T, int R, int C>
using Matrix = Eigen::Matrix<T, R, C>;

template <typename T, int R>
using Vector = Matrix<T, R, 1>;
template <typename T, int R>
using RowVector = Matrix<T, 1, R>;

using Vector3d = Vector<double, 3>;
using Vector4d = Vector<double, 4>;
using Vector6d = Vector<double, 6>;
}  // namespace art::eigen
