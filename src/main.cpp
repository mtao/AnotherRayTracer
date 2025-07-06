#include <iostream>

#include "art/Camera.hpp"
#include "art/geometry/Box.hpp"
#include "art/geometry/Sphere.hpp"
#include "art/objects/InternalSceneNode.hpp"
#include "art/objects/Object.hpp"

void sphere() {
    using namespace art;
    Camera cam(Camera::lookAt(/*position=*/Point(0, 0, 5),
                              /*looking_at=*/Point(0, 0, 0),
                              /*up=*/Point(0, 1, 0)));

    // Camera::perspective(
    //     /*fovy=*/70,
    //     /*aspect=*/1.,
    //     /*znear=*/0.1,
    //     /*zfar=*/10));
    objects::Object sphere(*std::make_shared<geometry::Sphere>());
    sphere.update_bounding_box();

    Image img = cam.render(20, 20, sphere);
}

void cube() {
    using namespace art;
    Camera cam(Camera::lookAt(/*position=*/Point(0, 0, 5),
                              /*looking_at=*/Point(0, 0, 0),
                              /*up=*/Point(0, 1, 0)));

    // Camera::perspective(
    //     /*fovy=*/70,
    //     /*aspect=*/1.,
    //     /*znear=*/0.1,
    //     /*zfar=*/10));
    objects::Object cube(*std::make_shared<geometry::Box>());

    cube.transform() = utils::AffineTransform::axis_angle_rotation(
        Vector3d({0., 0., 1.}), 1.8);
    cube.update_bounding_box();

    Image img = cam.render(20, 20, cube);
}

void both() {
    using namespace art;
    Camera cam(Camera::lookAt(/*position=*/Point(0, 0, 5),
                              /*looking_at=*/Point(0, 0, 0),
                              /*up=*/Point(0, 1, 0)));

    // Camera::perspective(
    //     /*fovy=*/70,
    //     /*aspect=*/1.,
    //     /*znear=*/0.1,
    //     /*zfar=*/10));
    auto cube =
        std::make_shared<objects::Object>(*std::make_shared<geometry::Box>());
    auto sphere = std::make_shared<objects::Object>(
        *std::make_shared<geometry::Sphere>());

    cube->transform() =
        utils::AffineTransform::translation(Vector3d({0., 1., 0.}));
    sphere->transform() =
        utils::AffineTransform::translation(Vector3d({0., -1., 0.}));

    auto scene = std::make_shared<objects::InternalSceneNode>();

    scene->add_node(cube);
    scene->add_node(sphere);

    scene->update_bounding_box();

    Image img = cam.render(100, 100, *scene);
}
int main(int argc, char* argv[]) {
    sphere();
    cube();
    both();
    return 0;
}
