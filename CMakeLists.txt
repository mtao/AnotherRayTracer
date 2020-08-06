cmake_minimum_required(VERSION 3.11.1)
project(AnotherRayTracer VERSION 0.1
    DESCRIPTION "Yet another raytracer"
    LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(art_fetch_dependencies)


OPTION(ART_USE_OPENGL "Enable the art opengl renderer" ON)

add_library(art
    src/geometry/bounding_box.cpp
    src/objects/sphere.cpp
    src/objects/scene_node.cpp
    src/camera.cpp
    src/point.cpp
    src/line.cpp
    src/rational.cpp
    src/geometry/ray.cpp
    )


target_include_directories(art PUBLIC include)

set_target_properties(art PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    )



target_link_libraries(art PUBLIC fmt)


add_executable( cmdline_art
    src/main.cpp
    )
target_link_libraries(cmdline_art PUBLIC art)
set_target_properties(cmdline_art PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    )
add_executable( rendergui_art
    src/rendergui.cpp
    )

target_link_libraries(rendergui_art PUBLIC art glfw OpenGL::GL ImGui::ImGui)
set_target_properties(rendergui_art PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    )
