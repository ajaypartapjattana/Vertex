#include "VulkanDescriptorPool.h"

#include "TypeMap/VulkanDescriptorTypeMap.h"

#include "VulkanDevice.h"
#include "VulkanDescriptorLayout.h"
#include "VulkanDescriptorSet.h"

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice* device, const VulkanDescriptorPoolDesc& desc)
	: device(device)
{
	std::vector<VkDescriptorPoolSize> sizes;
	sizes.reserve(desc.poolSizes.size());

	for (const auto& s : desc.poolSizes) {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = toVkDescriptorType(s.type);
		poolSize.descriptorCount = s.count;
	}

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	createInfo.maxSets = desc.maxSets;
	createInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
	createInfo.pPoolSizes = sizes.data();

	if (vkCreateDescriptorPool(device->getDevice(), &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

VulkanDescriptorPool::~VulkanDescriptorPool() {
	if (descriptorPool) {
		vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
	}
}

VulkanDescriptorSet VulkanDescriptorPool::allocate(const VulkanDescriptorSetLayout& layout) {
	VkDescriptorSetLayout vkLayout = layout.getHandle();

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &vkLayout;

	VkDescriptorSet set = VK_NULL_HANDLE;
	if (vkAllocateDescriptorSets(device->getDevice(), &allocInfo, &set) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	return VulkanDescriptorSet(set, &layout);
}

void VulkanDescriptorPool::reset() {
	vkResetDescriptorPool(device->getDevice(), descriptorPool, 0);
}

