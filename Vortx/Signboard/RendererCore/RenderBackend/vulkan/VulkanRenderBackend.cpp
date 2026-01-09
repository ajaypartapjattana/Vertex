#include "VulkanRenderBackend.h"

VulkanRenderBackend::VulkanRenderBackend(VulkanDevice& device, VulkanSurface& surface, uint32_t width, uint32_t height)
	: device(device), swapchain(device, width, height), renderGraph(device)
{
	createFrameContexts();
}

void VulkanRenderBackend::createFrameContexts() {
	frames.resize(FRAMES_IN_FLIGHT);

	for (FrameContext& frame : frames) {
		frame.commandBuffer = VulkanCommandBuffer(device, );
	}
}

