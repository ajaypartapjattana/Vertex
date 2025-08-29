#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

class ImGuiLayer
{
public:
	void init(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamily, VkQueue queue , VkRenderPass renderPass, uint32_t imageCount, GLFWwindow* window);

	void beginFrame();
	void endFrame(VkCommandBuffer commandBuffer);

	void cleanup(VkDevice device);

private:
	VkDescriptorPool imGuiDescriptorPool = VK_NULL_HANDLE;
};
