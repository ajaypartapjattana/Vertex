#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/PipelineTypes.h"

#include <vector>

class VulkanDevice;
class VulkanPipelineCache;
class VulkanPipelineLayout;
class VulkanRenderPass;

class VulkanPipeline {
public:
	VulkanPipeline(VulkanDevice& device, VulkanPipelineCache& cache);

	VulkanPipeline(const VulkanPipeline&) = delete;
	VulkanPipeline& operator=(const VulkanPipeline&) = delete;

	VulkanPipeline(VulkanPipeline&& other) noexcept;
	VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;

	~VulkanPipeline();

	void build(const VulkanRenderPass& renderPass, const VulkanPipelineLayout& layout, const PipelineDesc& desc);

	VkPipeline getHandle() const { return pipeline; }

private:

private:
	struct Impl;
	Impl* impl;

	VulkanDevice& device;
	VulkanPipelineCache& cache;

	VkPipeline pipeline = nullptr;
	bool built = false;
};