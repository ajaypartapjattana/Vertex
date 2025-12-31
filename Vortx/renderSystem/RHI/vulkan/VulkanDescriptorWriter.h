#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/DescriptorTypes.h"

#include <vector>

class VulkanDevice;
class VulkanDescriptorSet;
class VulkanBuffer;
class VulkanImage;

class VulkanDescriptorWriter {
public:
	VulkanDescriptorWriter(VulkanDevice* device, const VulkanDescriptorSet& set);

	VulkanDescriptorWriter& writeBuffer(uint32_t binding, const VulkanBuffer& buffer, DescriptorType type, uint64_t offset = 0, uint64_t range);
	VulkanDescriptorWriter& writeImage(uint32_t binding, const VulkanImage& image, DescriptorType type);

	void commit();

private:
	VulkanDevice* device = nullptr;
	VkDescriptorSet descriptorSet = nullptr;

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorBufferInfo> bufferInfos;
	std::vector<VkDescriptorImageInfo> imageInfos;

};