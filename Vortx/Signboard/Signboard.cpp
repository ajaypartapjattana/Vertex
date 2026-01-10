#include "Signboard.h"

#include <GLFW/glfw3.h>

Signboard::Signboard() 
	: window({ 1200, 800, "Signboard" }),
	  vulkanRHI(window.getWindowHandle()),
	  resources(vulkanRHI.getDevice()),
	  renderer(vulkanRHI.getRHIView(), resources.getResourceView(), resources.getSceneView())
{
	windowEvents.attachWindow(window.getWindowHandle());
	windowEvents.bindRenderer(&renderer);
}