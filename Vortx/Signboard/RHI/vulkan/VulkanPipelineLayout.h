#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/PipelineTypes.h"

#include <vector>

class VulkanDevice;
class VulkanDescriptorSetLayout;

class VulkanPipelineLayout {
public:
	VulkanPipelineLayout(VulkanDevice& device, const VulkanPipelineLayoutDesc& desc);
	~VulkanPipelineLayout();

	void addDescriptorSetLayout(VulkanDescriptorSetLayout& layout);
	void addPushConstantRange(PushConstantRangeDesc desc);

	void build();

	VkPipelineLayout getHandle() const { return layout; }

private:
	VulkanDevice& device;

	VkPipelineLayout layout = nullptr;
	bool built = false;

	struct Impl;
	Impl* impl;
};