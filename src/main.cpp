#include <iostream>

#include "art/Camera.hpp"
#include "art/objects/Object.hpp"
#include "art/geometry/Sphere.hpp"

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
    objects::Object sphere(*std::make_shared<geometry::Sphere>());
    sphere.update_bounding_box();

    Image img = cam.render(100, 100, sphere);
    return 0;
}
