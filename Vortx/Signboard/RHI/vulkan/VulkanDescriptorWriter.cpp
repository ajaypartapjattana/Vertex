#include "VulkanDescriptorWriter.h"

#include "TypeMap/VulkanDescriptorTypeMap.h"
#include "TypeMap/VulkanImageTypeMap.h"

#include "VulkanDevice.h"
#include "VulkanDescriptorSet.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"

struct VulkanDescriptorWriter::Impl {
	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorBufferInfo> bufferInfos;
	std::vector<VkDescriptorImageInfo> imageInfos;
};

VulkanDescriptorWriter::VulkanDescriptorWriter(VulkanDevice& device, const VulkanDescriptorSet& descriptorSet)
	: device(device), descriptorSet(descriptorSet)
{
	if (!descriptorSet.isValid()) {
		throw std::runtime_error("cannot write an invalid descriptor set!");
	}
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeCombinedImageSampler(uint32_t binding, const VulkanImage* image) {
	VkDescriptorImageInfo imageInfo{};
	if (image) {
		imageInfo.imageLayout = toVkImageLayout(image->getLayout());
		imageInfo.imageView = image->getView();
		imageInfo.sampler = image->getSampler();
	}

	impl->imageInfos.push_back(imageInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptorSet;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	write.pImageInfo = &impl->imageInfos.back();

	impl->writes.push_back(write);

	return *this;
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeUniformBuffer(uint32_t binding, const VulkanBuffer* buffer, uint64_t range = VK_WHOLE_SIZE, uint64_t offset = 0) {
	VkDescriptorBufferInfo bufferInfo{};
	if (buffer) {
		bufferInfo.buffer = buffer->getHandle();
		bufferInfo.offset = offset;
		bufferInfo.range = range;
	}

	impl->bufferInfos.push_back(bufferInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptorSet;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.descriptorCount = 1;
	write.pBufferInfo = &impl->bufferInfos.back();

	impl->writes.push_back(write);

	return *this;
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeStorageBuffer(uint32_t binding, const VulkanBuffer* buffer, uint64_t range = VK_WHOLE_SIZE, uint64_t offset = 0) {
	VkDescriptorBufferInfo bufferInfo{};
	if (buffer) {
		bufferInfo.buffer = buffer->getHandle();
		bufferInfo.offset = offset;
		bufferInfo.range = range;
	}

	impl->bufferInfos.push_back(bufferInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptorSet;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;;
	write.descriptorCount = 1;
	write.pBufferInfo = &impl->bufferInfos.back();

	impl->writes.push_back(write);

	return *this;
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeSampledImage(uint32_t binding, uint32_t arrayIndex, const VulkanImage* image) {
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageView = VK_NULL_HANDLE;
	if (image) {
		imageInfo.imageView = image->getView();
		imageInfo.imageLayout = toVkImageLayout(image->getLayout());
	}

	impl->imageInfos.push_back(imageInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptorSet;
	write.dstBinding = binding;
	write.dstArrayElement = arrayIndex;
	write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	write.descriptorCount = 1;
	write.pImageInfo = &impl->imageInfos.back();

	impl->writes.push_back(write);

	return *this;
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeSampler(uint32_t binding, uint32_t arrayIndex, const VulkanSampler* sampler) {
	VkDescriptorImageInfo imageInfo{};
	imageInfo.sampler = VK_NULL_HANDLE;
	if(sampler)
		imageInfo.sampler = sampler->getHandle();

	impl->imageInfos.push_back(imageInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptorSet;
	write.dstBinding = binding;
	write.dstArrayElement = arrayIndex;
	write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	write.descriptorCount = 1;
	write.pImageInfo = &impl->imageInfos.back();

	impl->writes.push_back(write);

	return *this;
}


void VulkanDescriptorWriter::commit() {
	if (impl->writes.empty()) {
		throw std::runtime_error("no writes to commit!");
	}

	vkUpdateDescriptorSets(device.getDevice(), static_cast<uint32_t>(impl->writes.size()), impl->writes.data(), 0, nullptr);

	impl->writes.clear();
	impl->bufferInfos.clear();
	impl->imageInfos.clear();
}