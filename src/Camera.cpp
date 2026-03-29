#include "art/Camera.hpp"

#include <zipper/transform/transform.hpp>

#include "art/Ray.hpp"
#include "art/utils/AffineTransform.hpp"
namespace art {

auto Camera::lookAt(const Point &position, const Point &target, const Point &up)
    -> utils::Isometry {
    Vector3d eye = position;
    Vector3d center = target;
    Vector3d up_vec = up;
    return zipper::transform::look_at(eye, center, up_vec);
}

auto Camera::render(size_t nx, size_t ny, objects::SceneNode &node) const
    -> Image {
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
            // Reset tMax for each new pixel ray
            ray.tMax = Rational(std::numeric_limits<double>::infinity());

            std::optional<Intersection> isect;
            if (node.intersect(ray, isect)) {
                // Normal-based coloring: map normal components to [0,1]
                auto n = isect->geometric_normal.normalized();
                float r = static_cast<float>(n(0) * 0.5 + 0.5);
                float g = static_cast<float>(n(1) * 0.5 + 0.5);
                float b = static_cast<float>(n(2) * 0.5 + 0.5);
                image.set_pixel(i, j, r, g, b, 1.f);
            } else {
                // Black for misses
                image.set_pixel(i, j, 0.f, 0.f, 0.f, 1.f);
            }
            image.increment_progress();
        }
    }
    return image;
}
} // namespace art
