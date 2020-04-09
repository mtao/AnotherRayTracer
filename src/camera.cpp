#include "art/camera.hpp"

#include <iostream>
namespace art {

Eigen::Matrix4d Camera::lookAt(const Point& position, const Point& target,
                               const Point& up) {
    Eigen::Matrix4d mat;
    mat.setZero();
    Eigen::Vector3d forward = (target - position).numerator();
    Eigen::Vector3d right = up.numerator().cross(forward);
    // TODO: figure out how to remove these normalizeds
    mat.block<1, 3>(0, 0) = right.transpose().normalized();
    mat.block<1, 3>(1, 0) =
        forward.cross(right).normalized();  // up.numerator().transpose();
    mat.block<1, 3>(2, 0) = forward.transpose().normalized();
    mat(3,3) = 1;

    Eigen::Matrix4d T = Eigen::Matrix4d::Zero();
    T.col(3) = -position;
    T.diagonal().array() = position.denominator();
    return mat * T;
}
//Eigen::Matrix4d Camera::perspective(const Rational& fovy,
//                                    const Rational& aspect,
//                                    const Rational& zNear,
//                                    const Rational& zFar) {
//    Eigen::Matrix4d R;
//    R.setZero();
//
//    Rational ymax = std::tan(double(fovy) * M_PI / 360.) * zNear;
//    Rational xmax = ymax * aspect;
//
//    Rational temp = 2.0 * zNear;
//    Rational temp2 = 2 * xmax;
//    Rational temp3 = 2 * ymax;
//    Rational temp4 = zFar - zNear;
//
//    R(0, 0) = double(temp / temp2);
//    R(1, 1) = double(temp / temp3);
//    R(2, 2) = double((-zNear - zFar) / temp3);
//    R(2, 3) = double(-temp * zFar / temp3);
//    R(3, 2) = -1.;
//    return R;
//}
Image Camera::render(size_t nx, size_t ny, objects::SceneNode& node) const {
    geometry::Ray ray;
    ray.origin = Point(0, 0, 0);


    Eigen::Matrix4d CI = _camera_transform.inverse();
    ray.origin.homogeneous() = CI * ray.origin.homogeneous();

    double dx = 1.0 / (nx-1);
    double dy = 1.0 / (ny-1);

    for (int j = 0; j < ny; ++j) {
        for (int i = 0; i < nx; ++i) {
            ray.direction(0) = dx * i - .5;
            ray.direction(1) = dy * j - .5;
            ray.direction(2) = -1;
            ray.direction =
                CI.topLeftCorner<3, 3>().transpose() * ray.direction;
            ;
            std::optional<Intersection> isect;
            //std::cout << std::string(ray.origin) << " => "
            //          << ray.direction.transpose() << std::endl;
            //if (node.intersect(ray, isect)) {
            //    std::cout <<"Hit!" << std::endl;
            //} else {
            //}
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
