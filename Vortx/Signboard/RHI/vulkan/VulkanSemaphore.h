#pragma once

#include "Common/VulkanFwd.h"

class VulkanDevice;

class VulkanSemaphore {
public:
	VulkanSemaphore(VulkanDevice& device);
	~VulkanSemaphore();

	VkSemaphore get() const { return semaphore; }

private:
	VulkanDevice& device;

	VkSemaphore semaphore = nullptr;
};