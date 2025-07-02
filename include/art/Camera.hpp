#pragma once

#include "art/Image.hpp"
#include "art/Point.hpp"
#include "art/objects/SceneNode.hpp"
#include "art/utils/AffineTransform.hpp"

namespace art {

class Camera {
   public:
    static utils::AffineTransform lookAt(const Point& position,
                                         const Point& target, const Point& up);
    static utils::AffineTransform perspective(const Rational& fovy,
                                              const Rational& aspect,
                                              const Rational& zNear,
                                              const Rational& zFar);
    // Camera(const Eigen::Matrix4d& ct, const Eigen::Matrix4d& pt)
    //     : _camera_transform(ct), _perspective_transform(pt) {}
    Camera(const utils::AffineTransform& ct) : _camera_transform(ct) {}

    Image render(size_t nx, size_t ny, objects::SceneNode& node) const;

    const utils::AffineTransform& transform() const {
        return _camera_transform;
    }

   private:
    utils::AffineTransform _camera_transform;
    // Point origin;
    // Point target;
    // Eigen::Vector3d up;
    // Eigen::Matrix4d _perspective_transform;
};
}  // namespace art
