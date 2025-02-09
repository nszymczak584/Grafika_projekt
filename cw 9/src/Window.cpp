#include "Window.h"

extern float aspectRatio;
extern int WIDTH, HEIGHT;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    aspectRatio = width / float(height);
    glViewport(0, 0, width, height);
    WIDTH = width;
    HEIGHT = height;
}