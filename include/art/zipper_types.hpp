#if !defined(ART_ZIPPER_TYPES_HPP)
#define ART_ZIPPER_TYPES_HPP

#include <zipper/Form.hpp>
#include <zipper/Matrix.hpp>
#include <zipper/Vector.hpp>
#include <zipper/concepts/Matrix.hpp>
#include <zipper/concepts/Vector.hpp>
#include <zipper/concepts/shapes.hpp>

namespace art {
template <typename T, int R, int C>
using Matrix = zipper::Matrix<T, R, C>;

template <typename T, int R>
using Vector = zipper::Vector<T, R>;
template <typename T, int R>
using RowVector = zipper::Form<T, R>;

using Vector3d = Vector<double, 3>;
using Vector4d = Vector<double, 4>;
using Vector6d = Vector<double, 6>;
using Matrix3d = Matrix<double, 3, 3>;
using Matrix4d = Matrix<double, 4, 4>;

template <typename T>
concept Matrix3x3Like = zipper::concepts::Matrix<T> &&
                        zipper::concepts::ValidExtents<T, 3, 3>;

template <typename T>
concept Vector3Like = zipper::concepts::Vector<T> &&
                      zipper::concepts::ValidExtents<T, 3>;

}  // namespace art
#endif
