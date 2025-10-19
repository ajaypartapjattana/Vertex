#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

class GuiLayer
{
public:
	void init(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamily, VkQueue queue , VkRenderPass renderPass, size_t imageCount, GLFWwindow* window);

	void beginFrame();
	void endFrame(VkCommandBuffer commandBuffer);

	void cleanup(VkDevice device);

private:
	VkDescriptorPool GuiDescriptorPool = VK_NULL_HANDLE;
};
