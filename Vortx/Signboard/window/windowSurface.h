#pragma once

#include <string>
#include <iostream>
#include <filesystem>

struct windowCreateInfo {
    uint32_t width;
    uint32_t height;
    const char* title;
};

class GLFWwindow;

class WindowSurface {
public:
    WindowSurface(const windowCreateInfo& info);
	~WindowSurface();

    GLFWwindow* getWindowHandle() const { return window; }

private:
    GLFWwindow* window;
};
