#include "VulkanSampler.h"

#include "TypeMap/VulkanSamplerTypeMap.h"

#include "VulkanDevice.h"

VulkanSampler::VulkanSampler(VulkanDevice& device, const SamplerDesc& desc)
	: device(device)
{
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	createInfo.magFilter = toVkFilter(desc.magFilter);
	createInfo.minFilter = toVkFilter(desc.minFilter);

	createInfo.addressModeU = toVkSamplerAddressMode(desc.addressU);
	createInfo.addressModeV = toVkSamplerAddressMode(desc.addressV);
	createInfo.addressModeW = toVkSamplerAddressMode(desc.addressW);
	
	createInfo.anisotropyEnable = desc.anisotropyEnable;
	createInfo.maxAnisotropy = desc.maxAnisotropy;

	createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.compareEnable = VK_FALSE;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(device.getDevice(), &createInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create sampler!");
	}
}

VulkanSampler::VulkanSampler(VulkanSampler&& other) noexcept 
	: device(other.device), sampler(other.sampler)
{
	other.sampler = VK_NULL_HANDLE;
}


VulkanSampler& VulkanSampler::operator=(VulkanSampler&& other) noexcept {
	if (this == &other)
		return *this;

	if (sampler)
		vkDestroySampler(device.getDevice(), sampler, nullptr);

	sampler = other.sampler;
	other.sampler = VK_NULL_HANDLE;

	return *this;
}

VulkanSampler::~VulkanSampler(){
	if (sampler) {
		vkDestroySampler(device.getDevice(), sampler, nullptr);
	}
}