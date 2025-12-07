#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "entityHandlers/renderer/VulkanContext.h"

class GuiLayer {
public:
	void init(ContextHandle handle, uint32_t queueFamily, size_t imageCount, GLFWwindow* window);

	void beginFrame();
	void endFrame(VkCommandBuffer commandBuffer);

	~GuiLayer();

private:
	VkDevice device = VK_NULL_HANDLE;
	VkDescriptorPool GuiDescriptorPool = VK_NULL_HANDLE;
};
