#pragma once

#include "Common/VulkanFwd.h"
#include "Signboard/RHI/common/DescriptorTypes.h"

class VulkanDevice;
class VulkanDescriptorSetLayout;
class VulkanDescriptorSet;

class VulkanDescriptorPool {
public:
	VulkanDescriptorPool(VulkanDevice& device, const DescriptorPoolDesc& desc);

	VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
	VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;

	VulkanDescriptorPool(VulkanDescriptorPool&&) = delete;
	VulkanDescriptorPool& operator=(VulkanDescriptorPool&&) = delete;

	~VulkanDescriptorPool();

	VulkanDescriptorSet allocate(const VulkanDescriptorSetLayout& layout, const uint32_t* variableCounts);

	void reset();

private:
	VulkanDevice& device;

	VkDescriptorPool descriptorPool = nullptr;
};