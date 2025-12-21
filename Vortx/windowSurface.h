#pragma once

#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <filesystem>

class WindowSurface {
public:
    WindowSurface(int w, int h, const char* title);
	~WindowSurface();

    GLFWwindow* handle;
};
