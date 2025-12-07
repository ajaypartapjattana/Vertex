#include "windowSurface.h"

WindowSurface::WindowSurface(int w, int h, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    handle = glfwCreateWindow(w, h, title, nullptr, nullptr);
    glfwMakeContextCurrent(handle);
}

WindowSurface::~WindowSurface() {
    glfwDestroyWindow(handle);
    glfwTerminate();
}