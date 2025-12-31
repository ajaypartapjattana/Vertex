#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/DescriptorTypes.h"

class VulkanDevice;
class VulkanDescriptorSetLayout;
class VulkanDescriptorSet;

class VulkanDescriptorPool {
public:
	VulkanDescriptorPool(VulkanDevice* device, const VulkanDescriptorPoolDesc& desc);

	VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
	VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;

	VulkanDescriptorPool(VulkanDescriptorPool&&) = delete;
	VulkanDescriptorPool& operator=(VulkanDescriptorPool&&) = delete;

	~VulkanDescriptorPool();

	VulkanDescriptorSet allocate(const VulkanDescriptorSetLayout& layout);

	void reset();

private:
	VulkanDevice* device = nullptr;

	VkDescriptorPool descriptorPool = nullptr;
};