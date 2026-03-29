#pragma once

#include "art/Image.hpp"
#include "art/Point.hpp"
#include "art/export.hpp"
#include "art/objects/SceneNode.hpp"
#include "art/utils/AffineTransform.hpp"

namespace art {

class ART_API Camera {
   public:
    static auto look_at(const Point &position,
                        const Point &target,
                        const Point &up) -> utils::Isometry;

    Camera(const utils::Isometry &ct) : m_camera_transform(ct) {}

    auto render(size_t nx, size_t ny, objects::SceneNode &node) const -> Image;

    auto transform() const -> const utils::Isometry & {
        return m_camera_transform;
    }

   private:
    utils::Isometry m_camera_transform;
};
} // namespace art
