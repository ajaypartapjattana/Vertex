#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/PipelineTypes.h"

#include <vector>

class VulkanDevice;
class VulkanDescriptorSetLayout;

class VulkanPipelineLayout {
public:
	VulkanPipelineLayout(VulkanDevice* device, const VulkanPipelineLayoutDesc& desc);

	VkPipelineLayout getHandle() const { return layout; }

private:
	VulkanDevice* device;

	VkPipelineLayout layout = nullptr;
	std::vector<VulkanDescriptorSetLayout> setLayouts;
};

class VulkanPipeline {
public:
	VulkanPipeline(VulkanDevice* device, const VulkanPipelineDesc& desc);

	VkPipeline getHandle() const { return pipeline; }


private:
	VulkanDevice* device = nullptr;

	VkPipeline pipeline = nullptr;
	VulkanPipelineLayout layout;
};