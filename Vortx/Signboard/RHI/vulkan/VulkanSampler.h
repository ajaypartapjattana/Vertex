#pragma once

#include "Common/VulkanFwd.h"
#include "Signboard/RHI/common/ImageSamplerTypes.h"

class VulkanDevice;

class VulkanSampler {
public:
	VulkanSampler(VulkanDevice& device, const SamplerDesc& desc);

	VulkanSampler(const VulkanSampler&) = delete;
	VulkanSampler& operator=(const VulkanSampler&) = delete;

	VulkanSampler(VulkanSampler&&) noexcept;
	VulkanSampler& operator=(VulkanSampler&&) noexcept;

	~VulkanSampler();

	VkSampler getHandle() const { return sampler; }

private:
	VulkanDevice& device;

	VkSampler sampler = nullptr;
};