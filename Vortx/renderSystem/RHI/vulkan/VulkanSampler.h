#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/ImageSamplerTypes.h"

class VulkanDevice;

class VulkanSampler {
public:
	VulkanSampler(VulkanDevice* device, const VulkanSamplerDesc& desc);

	VulkanSampler(const VulkanSampler&) = delete;
	VulkanSampler& operator=(const VulkanSampler&) = delete;

	~VulkanSampler();

	VkSampler getHandle() const { return sampler; }

private:
	VulkanDevice* device = nullptr;

	VkSampler sampler = nullptr;
};