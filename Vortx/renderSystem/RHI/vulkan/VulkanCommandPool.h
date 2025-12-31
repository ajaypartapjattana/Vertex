#pragma once

#include "Common/VulkanFwd.h"

class VulkanDevice;

class VulkanCommandPool {
public:
	explicit VulkanCommandPool(VulkanDevice* device);
	~VulkanCommandPool();

	VkCommandPool getHandle() const { return commandPool; }

private:
	VulkanDevice* device;

	VkCommandPool commandPool = nullptr;
};