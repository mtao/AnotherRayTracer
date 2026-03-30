#include "art/Camera.hpp"

#include <iostream>
#include <zipper/transform/all.hpp>

#include "art/Ray.hpp"
#include "art/utils/AffineTransform.hpp"
namespace art {

utils::Isometry Camera::lookAt(const Point& position, const Point& target,
                                const Point& up) {
    // Convert Points to Vector3d (divides numerator by denominator)
    Vector3d eye = position;
    Vector3d center = target;
    Vector3d up_vec = up;
    return zipper::transform::look_at(eye, center, up_vec);
}

Image Camera::render(size_t nx, size_t ny, objects::SceneNode& node) const {
    Ray ray;
    ray.origin = Point(0, 0, 0);

    auto CI = _camera_transform.inverse();

    ray.origin.homogeneous() = CI.to_matrix() * ray.origin.homogeneous();

    double dx = 1.0 / (nx - 1);
    double dy = 1.0 / (ny - 1);

    auto R_inv_t = CI.linear().transpose();

    for (size_t j = 0; j < ny; ++j) {
        for (size_t i = 0; i < nx; ++i) {
            ray.direction(0) = dx * i - .5;
            ray.direction(1) = dy * j - .5;
            ray.direction(2) = -1;
            ray.direction = R_inv_t * ray.direction;

            std::optional<Intersection> isect;
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
