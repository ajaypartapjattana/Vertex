#include "VulkanDescriptorWriter.h"

#include "TypeMap/VulkanDescriptorTypeMap.h"
#include "TypeMap/VulkanImageTypeMap.h"

#include "VulkanDevice.h"
#include "VulkanDescriptorSet.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

VulkanDescriptorWriter::VulkanDescriptorWriter(VulkanDevice* device, const VulkanDescriptorSet& set)
	: device(device), descriptorSet(set.getHandle())
{
	if (!set.isValid()) {
		throw std::runtime_error("cannot write an invalid descriptor set!");
	}
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeBuffer(uint32_t binding, const VulkanBuffer& buffer, DescriptorType type, uint64_t offset = 0, uint64_t range) {
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer.getHandle();
	bufferInfo.offset = offset;
	bufferInfo.range = range;

	bufferInfos.push_back(bufferInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptorSet;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = toVkDescriptorType(type);
	write.descriptorCount = 1;
	write.pBufferInfo = &bufferInfos.back();

	writes.push_back(write);

	return *this;
}
VulkanDescriptorWriter& VulkanDescriptorWriter::writeImage(uint32_t binding, const VulkanImage& image, DescriptorType type) {
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = toVkImageLayout(image.getLayout());
	imageInfo.imageView = image.getView();
	imageInfo.sampler = image.getSampler();

	imageInfos.push_back(imageInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptorSet;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = toVkDescriptorType(type);
	write.descriptorCount = 1;
	write.pImageInfo = &imageInfos.back();

	writes.push_back(write);

	return *this;
}

void VulkanDescriptorWriter::commit() {
	if (writes.empty()) {
		throw std::runtime_error("no writes to commit!");
	}

	vkUpdateDescriptorSets(device->getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	writes.clear();
	bufferInfos.clear();
	imageInfos.clear();
}