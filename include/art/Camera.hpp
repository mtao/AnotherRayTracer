#pragma once

#include "art/Image.hpp"
#include "art/Point.hpp"
#include "art/export.hpp"
#include "art/utils/AffineTransform.hpp"

namespace art::accel {
class SceneAccelerator;
}

namespace art {

class ART_API Camera {
  public:
    static auto lookAt(const Point &position,
                       const Point &target,
                       const Point &up) -> utils::Isometry;

    Camera(const utils::Isometry &ct) : _camera_transform(ct) {}

    auto render(size_t nx,
                size_t ny,
                const accel::SceneAccelerator &accelerator) const -> Image;

    auto transform() const -> const utils::Isometry & {
        return _camera_transform;
    }

  private:
    utils::Isometry _camera_transform;
};
} // namespace art
