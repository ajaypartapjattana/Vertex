#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/DescriptorTypes.h"

#include <vector>

class VulkanDevice;
class VulkanDescriptorSet;
class VulkanBuffer;
class VulkanImage;
class VulkanSampler;

class VulkanDescriptorWriter {
public:
	VulkanDescriptorWriter(VulkanDevice& device, const VulkanDescriptorSet& set);

	VulkanDescriptorWriter& writeCombinedImageSampler(uint32_t binding, const VulkanImage* image);
	VulkanDescriptorWriter& writeUniformBuffer(uint32_t binding, const VulkanBuffer* buffer, uint64_t range, uint64_t offset = 0);
	VulkanDescriptorWriter& writeStorageBuffer(uint32_t binding, const VulkanBuffer* buffer, uint64_t range, uint64_t offset = 0);
	VulkanDescriptorWriter& writeSampledImage(uint32_t binding, uint32_t index, const VulkanImage* image);
	VulkanDescriptorWriter& writeSampler(uint32_t binding, uint32_t index, const VulkanSampler* sampler);

	void commit();

private:
	VulkanDevice& device;

	const VulkanDescriptorSet& descriptorSet;

	struct Impl;
	Impl* impl;

};