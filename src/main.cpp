#include <iostream>

#include <zipper/transform/transform.hpp>

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

    objects::Object sphere(*std::make_shared<geometry::Sphere>());
    sphere.update_bounding_box();

    Image img = cam.render(20, 20, sphere);
}

void cube() {
    using namespace art;
    Camera cam(Camera::lookAt(/*position=*/Point(0, 0, 5),
                              /*looking_at=*/Point(0, 0, 0),
                              /*up=*/Point(0, 1, 0)));

    objects::Object cube(*std::make_shared<geometry::Box>());

    cube.transform() =
        zipper::transform::AxisAngleRotation<double>(1.8, {0., 0., 1.})
            .to_transform();
    cube.update_bounding_box();

    Image img = cam.render(20, 20, cube);
}

void both() {
    using namespace art;
    Camera cam(Camera::lookAt(/*position=*/Point(0, 0, 5),
                              /*looking_at=*/Point(0, 0, 0),
                              /*up=*/Point(0, 1, 0)));

    auto cube =
        std::make_shared<objects::Object>(*std::make_shared<geometry::Box>());
    auto sphere = std::make_shared<objects::Object>(
        *std::make_shared<geometry::Sphere>());

    cube->transform() =
        zipper::transform::Translation<double>(Vector3d({0., 1., 0.}))
            .to_transform();
    sphere->transform() =
        zipper::transform::Translation<double>(Vector3d({0., -1., 0.}))
            .to_transform();

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
