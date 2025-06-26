#include "art/camera.hpp"

#include <iostream>
#include <zipper/utils/inverse.hpp>
#include <zipper/views/nullary/ConstantView.hpp>
namespace art {

Matrix4d Camera::lookAt(const Point& position, const Point& target,
                        const Point& up) {
    Matrix4d mat;
    mat = zipper::views::nullary::ConstantView<double, 4, 4>(0);
    Vector3d forward = (target - position).numerator();
    Vector3d right = up.numerator().cross(forward);
    // TODO: figure out how to remove these normalizeds
    mat.row(0).head<3>() = right.normalized();
    mat.row(1).head<3>() = forward.cross(right).normalized();
    mat.row(2).head<3>() = forward.normalized();
    mat(3, 3) = 1;

    Matrix4d T = zipper::views::nullary::ConstantView<double, 4, 4>(0);
    T(0, 0) = 1;
    T(1, 1) = 1;
    T(2, 2) = 1;
    // mat = zipper::views::nullary::ConstantView<double, 3, 3>(0);
    T.col(3) = (-position).homogeneous();
    T.diagonal() = zipper::views::nullary::ConstantView(position.denominator());
    return mat * T;
}
// Matrix4d Camera::perspective(const Rational& fovy,
//                                     const Rational& aspect,
//                                     const Rational& zNear,
//                                     const Rational& zFar) {
//     Matrix4d R;
//     R.setZero();
//
//     Rational ymax = std::tan(double(fovy) * M_PI / 360.) * zNear;
//     Rational xmax = ymax * aspect;
//
//     Rational temp = 2.0 * zNear;
//     Rational temp2 = 2 * xmax;
//     Rational temp3 = 2 * ymax;
//     Rational temp4 = zFar - zNear;
//
//     R(0, 0) = double(temp / temp2);
//     R(1, 1) = double(temp / temp3);
//     R(2, 2) = double((-zNear - zFar) / temp3);
//     R(2, 3) = double(-temp * zFar / temp3);
//     R(3, 2) = -1.;
//     return R;
// }
Image Camera::render(size_t nx, size_t ny, objects::SceneNode& node) const {
    geometry::Ray ray;
    ray.origin = Point(0, 0, 0);

    // Matrix4d CI = _camera_transform.inverse();
    Matrix4d CI = zipper::views::nullary::ConstantView<double, 4, 4>(0);
    zipper::Matrix camera_rot = _camera_transform(zipper::static_slice<0, 3>(),
                                                  zipper::static_slice<0, 3>());
    zipper::Vector camera_t = _camera_transform.col(3).head<3>();
    auto R = CI(zipper::static_slice<0, 3>(), zipper::static_slice<0, 3>());

    R = zipper::utils::inverse(camera_rot);
    CI.col(3) = (-R * camera_t).homogeneous();

    ray.origin.homogeneous() = CI * ray.origin.homogeneous();

    double dx = 1.0 / (nx - 1);
    double dy = 1.0 / (ny - 1);

    for (size_t j = 0; j < ny; ++j) {
        for (size_t i = 0; i < nx; ++i) {
            ray.direction(0) = dx * i - .5;
            ray.direction(1) = dy * j - .5;
            ray.direction(2) = -1;
            ray.direction =
                CI(zipper::static_slice<0, 3>(), zipper::static_slice<0, 3>())
                    .transpose() *
                ray.direction;
            ;
            std::optional<Intersection> isect;
            // std::cout << std::string(ray.origin) << " => "
            //           << ray.direction.transpose() << std::endl;
            // if (node.intersect(ray, isect)) {
            //     std::cout <<"Hit!" << std::endl;
            // } else {
            // }
            if (node.intersect(ray, isect)) {
                std::cout << "o";
            } else {
                std::cout << ".";
            }
        }
        std::cout << std::endl;
    }
    return {};
}
}  // namespace art
