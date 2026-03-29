#if !defined(ART_CAMERA_HPP)
#define ART_CAMERA_HPP

#include "art/Image.hpp"
#include "art/Point.hpp"
#include "art/export.hpp"
#include "art/objects/SceneNode.hpp"
#include "art/utils/AffineTransform.hpp"

namespace art {

class ART_API Camera {
   public:
    static utils::Isometry lookAt(const Point& position, const Point& target,
                                  const Point& up);

    Camera(const utils::Isometry& ct) : _camera_transform(ct) {}

    Image render(size_t nx, size_t ny, objects::SceneNode& node) const;

    const utils::Isometry& transform() const { return _camera_transform; }

   private:
    utils::Isometry _camera_transform;
};
}  // namespace art
#endif
