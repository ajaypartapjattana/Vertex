#include "WindowEventProxy.h"

#include "Signboard/RendererCore/Renderer.h"
#include <GLFW/glfw3.h>

void WindowEventProxy::attachWindow(GLFWwindow* window)
{
    this->window = window;
	glfwSetWindowUserPointer(window, this);

	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetDropCallback(window, fileDropCallback);
}

void WindowEventProxy::bindRenderer(Renderer* r) {
    renderer = r;
}

void WindowEventProxy::onFrameBufferResize(
	int				width, 
	int				height) 
{
    if (renderer) {

    }
}

void WindowEventProxy::onFileDrop(
	int				count, 
	const char**	paths) 
{
    if (renderer) {

    }
}

static void framebufferResizeCallback(
    GLFWwindow*     window,
    int             width,
    int             height)
{
    auto* windowEvent = static_cast<WindowEventProxy*>(glfwGetWindowUserPointer(window));
    windowEvent->onFrameBufferResize(width, height);
}

static void fileDropCallback(
    GLFWwindow*     window,
    int             count,
    const char**    paths)
{
    auto* windowEvent = static_cast<WindowEventProxy*>(glfwGetWindowUserPointer(window));
    windowEvent->onFileDrop(count, paths);
}