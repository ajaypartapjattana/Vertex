#pragma once

#include "Common/VulkanFwd.h"

#include <vector>

struct GLFWwindow;

class VulkanDevice {
public:
	VulkanDevice(VkInstance instance, GLFWwindow* window);
	~VulkanDevice();

	void shutdown();

	VkDevice getDevice() const { return device; }
	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

	VkQueue getGraphicsQueue() const { return graphicsQueue; }
	uint32_t getGraphicsQueueFamily() const { return graphicsQueueFamily; }
	VkQueue getPresentQueue() const { return presentQueue; }
	uint32_t getPresentQueueFamily() const { return presentQueueFamily; }

	VkSurfaceKHR getSurface() const { return surface; }

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

private:
	void createSurface(GLFWwindow* window);
	void pickPhysicalDevice();
	void createLogicalDevice();

	bool isDeviceSuitable(VkPhysicalDevice device) const;
	void accquireQueueFamilies(VkPhysicalDevice physicalDevice);

private:
	VkInstance instance = nullptr;
	VkPhysicalDevice physicalDevice = nullptr;
	VkDevice device = nullptr;

	VkSurfaceKHR surface = nullptr;

	VkQueue graphicsQueue = nullptr;
	uint32_t graphicsQueueFamily = UINT32_MAX;

	VkQueue presentQueue = nullptr;
	uint32_t presentQueueFamily = UINT32_MAX;
};