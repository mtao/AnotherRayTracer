cmake_minimum_required(VERSION 3.11.1)

project(AnotherRayTracer VERSION 0.1
    DESCRIPTION "Yet another raytracer"
    LANGUAGES CXX)


add_executable(art
    src/geometry/bounding_box.cpp
    src/objects/sphere.cpp
    src/objects/scene_node.cpp
    src/main.cpp
    src/camera.cpp
    src/point.cpp
    src/rational.cpp
    src/geometry/ray.cpp

    )
target_include_directories(art PUBLIC include)

set_target_properties(art PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    )


find_package(FMT REQUIRED)

target_link_libraries(art PUBLIC fmt)

