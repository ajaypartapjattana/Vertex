#pragma once

#include "window/windowSurface.h"
#include "window/WindowEventProxy.h"
#include "RHI/VulkanRHI.h"
#include "resources/ResourceAPI.h"
#include "RendererCore/Renderer.h"

class GLFWwindow;

class Signboard {
public:
	Signboard();

private:
	WindowSurface window;
	WindowEventProxy windowEvents;
	
	VulkanRHI vulkanRHI;
	
	ResourceAPI resources;

	Renderer renderer;
};