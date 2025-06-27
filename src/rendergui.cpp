#include "art/Camera.hpp"
#include "art/objects/SceneNode.hpp"
#include "art/objects/Sphere.hpp"
#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <iostream>
#include <iostream>
#include <imgui.h>


using namespace art;
double zoom = 1.0;
Image make_image() {

    using namespace art;
    Camera cam(Camera::lookAt(/*position=*/Point(0, 0, 5),
                              /*looking_at=*/Point(0, 0, 0),
                              /*up=*/Point(0, 1, 0)));

               //Camera::perspective(
               //    /*fovy=*/70,
               //    /*aspect=*/1.,
               //    /*znear=*/0.1,
               //    /*zfar=*/10));
    objects::SceneNode scene;
    scene.add_node(std::make_shared<objects::Sphere>());

    Image img = cam.render(300,300, scene);
    return img;
}
void resize(GLFWwindow* window, int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0,0,width,height);
    double scale = zoom * double(height) / width;
    glOrtho(-zoom,zoom,-scale,scale,-1,1);

}

int main(int argc, char* argv[]) {
    GLFWwindow* window;
    if(!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(640,480, "Art Rendergui", nullptr,nullptr);
    if(!window) {
        glfwTerminate();
        return -1;
    }
    glPushMatrix();
    glfwMakeContextCurrent(window);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    //glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, filename);
    glfwSetWindowSizeCallback(window,resize);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);


        glBegin(GL_QUADS);
        glColor3f(1,0,0);
        glTexCoord2f(0,0); 
        glVertex2f(-.5,-.5);
        glTexCoord2f(0,1); 
        glVertex2f(-.5,.5);
        glTexCoord2f(1,1);
        glVertex2f(.5,.5);
        glTexCoord2f(1,0);
        glVertex2f(.5,-.5);
        glEnd();



        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glPushMatrix();
    glfwTerminate();
    return 0;
}
