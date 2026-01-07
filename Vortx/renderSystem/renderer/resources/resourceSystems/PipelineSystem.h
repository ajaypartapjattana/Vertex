#pragma once

#include "renderSystem/renderer/resources/common/PipelineSystemTypes.h"
#include "renderSystem/RHI/common/PipelineTypes.h"
#include "ResourceHash/PipelineHash.h"

#include "renderSystem/RHI/vulkan/VulkanPipelineCache.h"

#include <memory>
#include <unordered_map>

class VulkanDevice;
class VulkanPipelineLayout;
class VulkanPipeline;
class VulkanRenderPass;

class VulkanPipelineCache;

class PipelineSystem {
	explicit PipelineSystem(VulkanDevice& device);
	~PipelineSystem();

	PipelineHandle getOrCreatePipleine(VulkanPipelineLayout& layout, const PipelineDesc& desc, const VulkanRenderPass& renderPass);

	const VulkanPipeline& get(PipelineHandle handle) const;

	void destroy(PipelineHandle handle);
	void flushDeletes();

private:
	PipelineHandle allocateSlot(std::unique_ptr<VulkanPipeline> pipeline, const PipelineKey& key);

	PipelineKey makeKey(VulkanPipelineLayout& layout, const PipelineDesc& desc, const VulkanRenderPass& renderPass);

private:
	VulkanDevice& device;

	VulkanPipelineCache cache;

	struct PipelineSlot {
		std::unique_ptr<VulkanPipeline> pipeline;
		PipelineKey key;
		uint32_t generation = 0;
	};

	std::vector<PipelineSlot> slots;
	std::vector<uint32_t> freeList;

	std::unordered_map<PipelineKey, uint32_t, PipelineKeyHash> pipelineLookup;

	std::vector<uint32_t> pendingDeletes;

};