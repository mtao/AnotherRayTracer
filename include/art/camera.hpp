#pragma once

#include "art/image.hpp"
#include "art/objects/scene_node.hpp"
#include "art/point.hpp"

namespace art {

class Camera {
   public:
    static Matrix4d lookAt(const Point& position, const Point& target,
                           const Point& up);
    // static Eigen::Matrix4d perspective(const Rational& fovy,
    //                                    const Rational& aspect,
    //                                    const Rational& zNear,
    //                                    const Rational& zFar);
    // Camera(const Eigen::Matrix4d& ct, const Eigen::Matrix4d& pt)
    //     : _camera_transform(ct), _perspective_transform(pt) {}
    Camera(const Matrix4d& ct) : _camera_transform(ct) {}

    Image render(size_t nx, size_t ny, objects::SceneNode& node) const;

    const Matrix4d& transform() const {
        return _camera_transform;
    }

   private:
    Matrix4d _camera_transform;
    // Point origin;
    // Point target;
    // Eigen::Vector3d up;
    // Eigen::Matrix4d _perspective_transform;
};
}  // namespace art
