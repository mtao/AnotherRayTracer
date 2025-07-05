#include "art/Camera.hpp"

#include <iostream>
#include <zipper/utils/inverse.hpp>
#include <zipper/views/nullary/ConstantView.hpp>
#include <zipper/views/nullary/IdentityView.hpp>

#include "art/Ray.hpp"
#include "art/utils/AffineTransform.hpp"
namespace art {

utils::AffineTransform Camera::lookAt(const Point& position,
                                      const Point& target, const Point& up) {
    utils::AffineTransform mat;
    Vector3d forward = (target - position).numerator();
    Vector3d right = up.numerator().cross(forward);
    auto R = mat.rotation();
    // TODO: figure out how to remove these normalizeds
    R.row(0) = right.normalized();
    R.row(1) = forward.cross(right).normalized();
    R.row(2) = forward.normalized();

    auto t = mat.translation();

    t = -position.homogeneous();
    return mat;
}
utils::AffineTransform Camera::perspective(const Rational& fovy,
                                           const Rational& aspect,
                                           const Rational& zNear,
                                           const Rational& zFar) {
    utils::AffineTransform R = zipper::views::nullary::ConstantView<double>(0);

    // TODO: try to recall where this commented impl of perspective comes from, equation seems nice but would like to verfiy
    /*
    Rational ymax = std::tan(double(fovy) * M_PI / 360.) * zNear;
    Rational xmax = ymax * aspect;

    Rational temp = 2.0 * zNear;
    Rational temp2 = 2 * xmax;
    Rational temp3 = 2 * ymax;
    Rational temp4 = zFar - zNear;

    R(0, 0) = double(temp / temp2);
    R(1, 1) = double(temp / temp3);
    R(2, 2) = double((-zNear - zFar) / temp3);
    R(2, 3) = double(-temp * zFar / temp3);
    R(3, 2) = -1.;
    */
    // https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
    double f = atan(double(fovy/2.0));
    R(0, 0) = double(f / aspect);
    R(1, 1) = double(f);
    R(2, 2) = double((zFar + zNear) / (zNear - zFar));
    R(3, 2) = -1;
    R(2, 3) = double(2 * zFar * zNear / (zNear - zFar));

    return R;
}
Image Camera::render(size_t nx, size_t ny, objects::SceneNode& node) const {
    Ray ray;
    ray.origin = Point(0, 0, 0);

    // Matrix4d CI = _camera_transform.inverse();
    Matrix4d CI =
        // utils::AffineTransform CI =
        zipper::views::nullary::ConstantView<double, 4, 4>(0);
    zipper::MatrixBase camera_rot = _camera_transform.rotation();
    zipper::Vector camera_t = _camera_transform.translation().head<3>();
    auto R = CI(zipper::static_slice<0, 3>(), zipper::static_slice<0, 3>());

    R = zipper::utils::inverse(camera_rot);
    CI.col(3) = (-R * camera_t).homogeneous();

    // utils::AffineTransform CI;
    // zipper::MatrixBase camera_rot = _camera_transform.rotation();
    // auto camera_t = _camera_transform.translation();
    // auto R = CI.rotation();

    // R = zipper::utils::inverse(camera_rot);
    // CI.translation().head<3>() = (R * camera_t.head<3>());
    // CI.translation()(3) = -camera_t(3);

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
