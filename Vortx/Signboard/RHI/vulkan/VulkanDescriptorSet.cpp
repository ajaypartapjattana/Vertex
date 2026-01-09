#include "VulkanDescriptorSet.h"

#include "Common/VulkanCommon.h"
#include "VulkanDescriptorLayout.h"

VulkanDescriptorSet::VulkanDescriptorSet(VkDescriptorSet descriptorSet, const VulkanDescriptorSetLayout* layout)
	: descriptorSet(descriptorSet), layout(layout) {}

const VulkanDescriptorSetLayout& VulkanDescriptorSet::getLayout() const {
	if (!layout) {
		throw std::runtime_error("descriptor set no layout!");
	}
	return *layout;
}
