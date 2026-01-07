#include "SamplerSystem.h"

#include "renderSystem/RHI/vulkan/VulkanSampler.h"

#include "renderSystem/RHI/vulkan/VulkanDescriptorSet.h"
#include "renderSystem/RHI/vulkan/VulkanDescriptorWriter.h"

#include <stdexcept>
#include <cassert>

SamplerSystem::SamplerSystem(VulkanDevice& device, VulkanDescriptorSet& samplerSet, uint32_t samplerBindingIndex, uint32_t maxSamplerCount) 
	: device(device), samplerSet(samplerSet), samplerBindingIndex(samplerBindingIndex), maxSamplerCount(maxSamplerCount) {}

SamplerSystem::~SamplerSystem() {
	slots.clear();
	freeList.clear();
}

SamplerHandle SamplerSystem::createSampler(const SamplerDesc& desc) {
	if (slots.size() >= maxSamplerCount)
		throw std::runtime_error("out of bindless samplers slots!");

	auto sampler = std::make_unique<VulkanSampler>(device, desc);

	SamplerHandle handle =  allocateSlot(std::move(sampler));
	writeSamplerDescriptor(handle);

	return handle;
}

void SamplerSystem::writeSamplerDescriptor(const SamplerHandle handle) {
	VulkanDescriptorWriter writer(device, samplerSet);

	const VulkanSampler& sampler = get(handle);
	writer.writeSampler(samplerBindingIndex, handle.index, &sampler);

	writer.commit();
}

void SamplerSystem::clearSamplerDescriptor(const SamplerHandle handle) {
	VulkanDescriptorWriter writer(device, samplerSet);

	writer.writeSampler(samplerBindingIndex, handle.index, nullptr);

	writer.commit();
}

SamplerHandle SamplerSystem::allocateSlot(std::unique_ptr<VulkanSampler> sampler) {
	uint32_t index;

	if (!freeList.empty()) {
		index = freeList.back();
		freeList.pop_back();
	}
	else {
		index = static_cast<uint32_t>(slots.size());
		slots.emplace_back();
	}

	Slot& slot = slots[index];
	slot.sampler = std::move(sampler);

	return SamplerHandle{ index, slot.generation };
}

const VulkanSampler& SamplerSystem::get(SamplerHandle handle) const {
	assert(handle.index < slots.size());

	const Slot& slot = slots[handle.index];

	assert(slot.sampler && "SamplerHandle refers to destroyed sampler!");
	assert(slot.generation == handle.generation && "SamplerHandle generation mismatch!");

	return *slot.sampler;
}

void SamplerSystem::destroy(SamplerHandle handle) {
	uint32_t index = handle.index;
	if (index == INVALID_SAMPLER.index)
		return;
	assert(index < slots.size());

	Slot& slot = slots[index];
	if (slot.generation != handle.generation)
		return;

	pendingDeletes.push_back(index);
}

void SamplerSystem::flushDeletes() {
	for (uint32_t index : pendingDeletes) {
		Slot& slot = slots[index];

		if (!slot.sampler) continue;
		slot.sampler.reset();
		++slot.generation;
		freeList.push_back(index);
	}
	pendingDeletes.clear();
}