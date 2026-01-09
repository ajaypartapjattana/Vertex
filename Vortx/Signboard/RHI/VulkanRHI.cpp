#include "VulkanRHI.h"

VulkanRHI::VulkanRHI(GLFWwindow* window)
	: vulkanContext(window),
	  device(vulkanContext.getInstance(), window),
	
	  swapchain(device, 1200, 1200),

	  graphicsCommandPool(device)
{
	
}