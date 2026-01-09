#pragma once

#include "Signboard/resources/common/SamplerSystemTypes.h"

#include <vector>
#include <memory>

class VulkanDevice;
class VulkanSampler;

class VulkanDescriptorSet;

class SamplerSystem{
public:
	explicit SamplerSystem(VulkanDevice& device, VulkanDescriptorSet& samplerSet, uint32_t samplerBindingIndex, uint32_t maxSampelrCount);
	~SamplerSystem();

	SamplerHandle createSampler(const SamplerDesc& desc);
	const VulkanSampler& get(SamplerHandle handle) const;

	void destroy(SamplerHandle handle);
	void flushDeletes();

private:
	void writeSamplerDescriptor(const SamplerHandle handle);
	void clearSamplerDescriptor(const SamplerHandle handle);

	SamplerHandle allocateSlot(std::unique_ptr<VulkanSampler> sampler);

private:
	VulkanDevice& device;

	VulkanDescriptorSet& samplerSet;

	uint32_t samplerBindingIndex;
	uint32_t maxSamplerCount;

	struct Slot {
		std::unique_ptr<VulkanSampler> sampler;
		uint32_t generation;
	};

	std::vector<Slot> slots;
	std::vector<uint32_t> freeList;

	std::vector<uint32_t> pendingDeletes;

};

