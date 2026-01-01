#include "VulkanDescriptorLayout.h"

#include "TypeMap/VulkanDescriptorTypeMap.h"

#include "VulkanDevice.h"

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice& device, const VulkanDescriptorSetLayoutDesc& desc)
	: device(device)
{
	std::vector<VkDescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(desc.bindings.size());
	for (const auto& b : desc.bindings) {
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = b.binding;
		binding.descriptorType = toVkDescriptorType(b.type);
		binding.descriptorCount = b.count;
		binding.stageFlags = toVkShaderStageFlags(b.stages);
		binding.pImmutableSamplers = nullptr;

		vkBindings.push_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
	createInfo.pBindings = vkBindings.data();

	if (vkCreateDescriptorSetLayout(device.getDevice(), &createInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept
	: device(other.device), descriptorSetLayout(other.descriptorSetLayout)
{
	other.descriptorSetLayout = nullptr;
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
	if (descriptorSetLayout) {
		vkDestroyDescriptorSetLayout(device.getDevice(), descriptorSetLayout, nullptr);
	}
}

