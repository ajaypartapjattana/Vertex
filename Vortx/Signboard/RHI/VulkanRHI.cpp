#include "VulkanRHI.h"

VulkanRHI::VulkanRHI(GLFWwindow* window)
	: vulkanContext(window),
	  device(vulkanContext.getInstance(), window),
	
	  swapchain(device, 1200, 1200),

	  graphicsCommandPool(device)
{
	
}

VulkanRHI::~VulkanRHI() {
	device.waitIdle();
}

RHIView VulkanRHI::getRHIView() {
	return RHIView{
		device,
		swapchain,
		graphicsCommandPool
	};
}