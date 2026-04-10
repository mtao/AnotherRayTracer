#include "art/Camera.hpp"

#include <cmath>

#include <zipper/transform/all.hpp>

#include "art/Ray.hpp"
#include "art/utils/AffineTransform.hpp"

namespace art {

utils::Isometry Camera::lookAt(const Point &position,
                               const Point &target,
                               const Point &up) {
    // Convert Points to Vector3d (divides numerator by denominator)
    Vector3d eye = position;
    Vector3d center = target;
    Vector3d up_vec = up;
    return zipper::transform::look_at(eye, center, up_vec);
}

Image Camera::render(size_t nx, size_t ny, objects::SceneNode &node) const {
    Image image(nx, ny);

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
                // Headlight shading: abs(N . view_dir) gives a basic
                // diffuse look without requiring separate light sources.
                double dir_len = ray.direction.norm<2>();
                Vector3d n = isect->normal.normalized();
                float shade = static_cast<float>(
                    std::abs(n.dot(ray.direction) / dir_len));
                image.set_pixel(i, j, shade, shade, shade, 1.f);
            } else {
                image.set_pixel(i, j, 0.f, 0.f, 0.f, 1.f);
            }
            image.increment_progress();
        }
    }
    return image;
}

} // namespace art
