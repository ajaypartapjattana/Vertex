#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/DescriptorTypes.h"

class VulkanDevice;

class VulkanDescriptorSetLayout {
public:
	VulkanDescriptorSetLayout(VulkanDevice& device, const VulkanDescriptorSetLayoutDesc& desc);

	VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
	VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

	VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept;
	VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& other) = delete;

	~VulkanDescriptorSetLayout();

	VkDescriptorSetLayout getHandle() const { return descriptorSetLayout; }

private:
	VulkanDevice& device;

	VkDescriptorSetLayout descriptorSetLayout = nullptr;
};