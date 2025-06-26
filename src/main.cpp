#include <iostream>

#include "art/camera.hpp"
#include "art/objects/scene_node.hpp"
#include "art/objects/sphere.hpp"

int main(int argc, char* argv[]) {
    using namespace art;
    Camera cam(Camera::lookAt(/*position=*/Point(0, 0, 5),
                              /*looking_at=*/Point(0, 0, 0),
                              /*up=*/Point(0, 1, 0)));

    // Camera::perspective(
    //     /*fovy=*/70,
    //     /*aspect=*/1.,
    //     /*znear=*/0.1,
    //     /*zfar=*/10));
    objects::SceneNode scene;
    scene.add_node(std::make_shared<objects::Sphere>());

    Image img = cam.render(100, 100, scene);
    return 0;
}
