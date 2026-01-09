#pragma once

#include "Common/VulkanFwd.h"

class VulkanDescriptorSetLayout;

class VulkanDescriptorSet {
public:
	VulkanDescriptorSet() = default;

	VulkanDescriptorSet(VkDescriptorSet descriptorSet, const VulkanDescriptorSetLayout* layout);

	VkDescriptorSet getHandle() const { return descriptorSet; }
	const VulkanDescriptorSetLayout& getLayout() const;

	operator VkDescriptorSet() const { return descriptorSet; }

	bool isValid() const { return descriptorSet != nullptr; }

private:
	VkDescriptorSet descriptorSet = nullptr;
	const VulkanDescriptorSetLayout* layout = nullptr;
};