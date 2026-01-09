#include "windowSurface.h"

#include <GLFW/glfw3.h>

WindowSurface::WindowSurface(
    const windowCreateInfo& info) 
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
    glfwMakeContextCurrent(window);
}

WindowSurface::~WindowSurface() 
{
    glfwDestroyWindow(window);
    glfwTerminate();
}