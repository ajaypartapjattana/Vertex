#include "Signboard.h"

#include <GLFW/glfw3.h>

Signboard::Signboard() 
	: window({ 1200, 800, "Signboard" }),
	  vulkanRHI(window.getWindowHandle()),
	  resources(vulkanRHI.getDevice()),
	  renderer(vulkanRHI.getDevice(), resources.getResourceView())
{
	windowEvents.attachWindow(window.getWindowHandle());
	windowEvents.bindRenderer(&renderer);
}