#pragma once

#include "vulkan/VulkanContext.h"
#include "vulkan/VulkanDevice.h"

#include "vulkan/VulkanSwapchain.h"

#include "vulkan/VulkanCommandPool.h"

struct RHIView {
	VulkanDevice& device;
	VulkanSwapchain& swapchain;
	VulkanCommandPool& commandPool;
};

class GLFWwindow;

class VulkanRHI {
public:
	VulkanRHI(GLFWwindow* window);
	~VulkanRHI();

	RHIView getRHIView();

	VulkanDevice& getDevice() { return device; }

private:
	VulkanContext vulkanContext;
	VulkanDevice device;

	VulkanSwapchain swapchain;

	VulkanCommandPool graphicsCommandPool;
};