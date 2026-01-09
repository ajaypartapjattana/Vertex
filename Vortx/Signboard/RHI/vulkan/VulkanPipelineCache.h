#pragma once

#include "Common/VulkanFwd.h"

#include <memory>

class VulkanDevice;

class VulkanPipelineCache {
public:
	VulkanPipelineCache(VulkanDevice& device);
	~VulkanPipelineCache();

	void saveToDisk(const char* path);
	void loadFromDisk(const char* path);

	VkPipelineCache getHandle() const { return cache; }

private:
	VulkanDevice& device;

	VkPipelineCache cache = nullptr;
};