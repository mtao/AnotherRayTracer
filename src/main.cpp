#include <iostream>

#include <zipper/transform/transform.hpp>

#include "art/Camera.hpp"
#include "art/accel/LinearAccelerator.hpp"
#include "art/geometry/Box.hpp"
#include "art/geometry/Sphere.hpp"
#include "art/io/image_io.hpp"
#include "art/objects/InternalSceneNode.hpp"
#include "art/objects/Object.hpp"

void sphere() {
    using namespace art;
    Camera cam(Camera::lookAt(/*position=*/Point(0, 0, 5),
                              /*looking_at=*/Point(0, 0, 0),
                              /*up=*/Point(0, 1, 0)));

    auto obj = std::make_shared<objects::Object>(
        *std::make_shared<geometry::Sphere>());

    accel::LinearAccelerator accel;
    accel.build(*obj);

    Image img = cam.render(20, 20, accel);
}

void cube() {
    using namespace art;
    Camera cam(Camera::lookAt(/*position=*/Point(0, 0, 5),
                              /*looking_at=*/Point(0, 0, 0),
                              /*up=*/Point(0, 1, 0)));

    auto obj =
        std::make_shared<objects::Object>(*std::make_shared<geometry::Box>());

    obj->transform() =
        zipper::transform::AxisAngleRotation<double>(1.8, {0., 0., 1.})
            .to_transform();

    accel::LinearAccelerator accel;
    accel.build(*obj);

    Image img = cam.render(20, 20, accel);
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

    accel::LinearAccelerator accel;
    accel.build(*scene);

    Image img = cam.render(100, 100, accel);
    auto result = art::io::save("both.ppm", img);
    if (!result) {
        std::cerr << "Failed to save: " << result.error() << std::endl;
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    sphere();
    cube();
    both();
    return 0;
}
