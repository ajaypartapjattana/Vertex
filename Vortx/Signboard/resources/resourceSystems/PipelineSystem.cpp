#include "PipelineSystem.h"

#include "Signboard/RHI/vulkan/VulkanPipeline.h"
#include "Signboard/RHI/vulkan/VulkanPipelineLayout.h"
#include "Signboard/RHI/vulkan/VulkanPipelineCache.h"

#include "Signboard/RHI/vulkan/VulkanRenderPass.h"

#include"ResourceHash/PipelineHash.h"

#include <cassert>

PipelineSystem::PipelineSystem(VulkanDevice& device) 
	: device(device), cache(device)
{

}

PipelineSystem::~PipelineSystem() {
	slots.clear();
	freeList.clear();
}

PipelineHandle PipelineSystem::getOrCreatePipleine(VulkanPipelineLayout& layout, const PipelineDesc& desc, const VulkanRenderPass& renderPass) {
	PipelineKey key = makeKey(layout, desc, renderPass);

	auto it = pipelineLookup.find(key);
	if (it != pipelineLookup.end())
		return PipelineHandle{ it->second, slots[it->second].generation };

	auto pipeline = std::make_unique<VulkanPipeline>(device, cache);
	pipeline->build(renderPass, layout, desc);

	PipelineHandle handle = allocateSlot(std::move(pipeline), key);
	pipelineLookup.emplace(key, handle.index);

	return handle;
}

PipelineHandle PipelineSystem::allocateSlot(std::unique_ptr<VulkanPipeline> pipeline, const PipelineKey& key) {
	uint32_t index;

	if (!freeList.empty()) {
		index = freeList.back();
		freeList.pop_back();
	} else {
		index = static_cast<uint32_t>(slots.size());
		slots.emplace_back();
	}

	PipelineSlot& slot = slots[index];
	slot.pipeline = std::move(pipeline);
	slot.key = key;
	slot.generation++;

	return PipelineHandle{ index, slot.generation };
}

const VulkanPipeline& PipelineSystem::get(PipelineHandle handle) const {
	assert(handle.index < slots.size());

	const PipelineSlot& slot = slots[handle.index];
	assert(slot.pipeline);
	assert(slot.generation == handle.generation);
	
	return *slot.pipeline;
}

void PipelineSystem::destroy(PipelineHandle handle) {
	uint32_t index = handle.index;
	if (index >= slots.size())
		return;

	if (slots[index].generation != handle.generation)
		return;

	pendingDeletes.push_back(index);
}

void PipelineSystem::flushDeletes() {
	for (uint32_t index : pendingDeletes) {
		PipelineSlot& slot = slots[index];

		if (!slot.pipeline) continue;
		pipelineLookup.erase(slot.key);
		slot.pipeline.reset();
		slot.generation++;
		freeList.push_back(index);
	}
	pendingDeletes.clear();
}

PipelineKey PipelineSystem::makeKey(VulkanPipelineLayout& layout, const PipelineDesc& desc, const VulkanRenderPass& renderPass) {
	PipelineKey key{};

	key.type = desc.type;

	key.layout = layout.getHandle();
	key.renderPass = renderPass.getHandle();

	key.colorFormat = desc.colorFormat;
	key.depthForamt = desc.depthForamt;

	key.shadersHashes.reserve(desc.shaders.size());
	for (const ShaderDesc& shader : desc.shaders) {
		key.shadersHashes.push_back(hashShaderFile(shader.path));
	}

	key.raster = desc.raster;
	key.blend = desc.blend;

	return key;
}