#include "VulkanImage.h"

#include "TypeMap/VulkanImageTypeMap.h"

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSampler.h"
#include "VulkanBuffer.h"

VkImageAspectFlags chooseAspectMask(ImageFormat format, ImageLayout layout) {
	if (layout == ImageLayout::DepthStencilAttachment) {
		VkImageAspectFlags aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencilComponent(format)) {
			aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		return aspect;
	}
	return VK_IMAGE_ASPECT_COLOR_BIT;
}

void getAccessFlags(ImageLayout oldLayout, ImageLayout newLayout, VkAccessFlags& srcAccess, VkAccessFlags& dstAccess) {
	if (oldLayout == ImageLayout::Undefined && newLayout == ImageLayout::TransferDst) {
		dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if (oldLayout == ImageLayout::TransferDst && newLayout == ImageLayout::ShaderReadOnly) {
		srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstAccess = VK_ACCESS_SHADER_READ_BIT;
	}
	else if (oldLayout == ImageLayout::Undefined &&	newLayout == ImageLayout::DepthStencilAttachment) {
		dstAccess =	VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else if (oldLayout == ImageLayout::Undefined && newLayout == ImageLayout::ColorAttachment) {
		dstAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
}

VulkanImage::VulkanImage(VulkanDevice& device, const VulkanImageDesc& desc, VulkanSampler* sampler = nullptr)
	: device(device), sampler(sampler)
{
	format = desc.format;
	extent = { desc.width, desc.height };

	createImage(extent, desc.usage);
	allocateMemory();
	createImageView(desc.usage);
}

VulkanImage::VulkanImage(VulkanImage&& other) noexcept
	: device(other.device), image(other.image), memory(other.memory), imageView(other.imageView)
{
	format = other.format;
	layout = other.layout;

	other.image = VK_NULL_HANDLE;
	other.memory = VK_NULL_HANDLE;
	other.imageView = VK_NULL_HANDLE;
}

VulkanImage& VulkanImage::operator=(VulkanImage&& other) noexcept {
	if (this == &other) {
		return *this;
	}
	destroy();

	device = other.device;
	image = other.image;
	memory = other.memory;
	imageView = other.imageView;
	format = other.format;
	layout = other.layout;

	other.image = nullptr;
	other.memory = nullptr;
	other.imageView = nullptr;
}


VulkanImage::~VulkanImage() {
	destroy();
}

void VulkanImage::destroy() {
	VkDevice vkDevice = device.getDevice();

	if (imageView) {
		vkDestroyImageView(vkDevice, imageView, nullptr);
	}
	if (image) {
		vkDestroyImage(vkDevice, image, nullptr);
	}
	if (memory) {
		vkFreeMemory(vkDevice, memory, nullptr);
	}
}

void VulkanImage::copyFromBuffer(VulkanCommandBuffer& cmd, const VulkanBuffer& src) {
	if (layout != ImageLayout::TransferDst)
		transitionLayout(cmd, ImageLayout::TransferDst, PipelineStage::TopOfPipe, PipelineStage::Transfer);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = chooseAspectMask(format, layout);
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0,0,0 };
	region.imageExtent = { extent.width, extent.height, 1 };

	vkCmdCopyBufferToImage(cmd.getHandle(), src.getHandle(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanImage::assignSampler(VulkanSampler* s) {
	sampler = s;
}

VkSampler VulkanImage::getSampler() const {
	return sampler ? sampler->getHandle() : VK_NULL_HANDLE;
}

void VulkanImage::transitionLayout(VulkanCommandBuffer& cmd, ImageLayout newLayout, PipelineStageFlags srcStage, PipelineStageFlags dstStage) {
	VkImageLayout oldVkLayout = toVkImageLayout(layout);
	VkImageLayout newVkLayout = toVkImageLayout(newLayout);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldVkLayout;
	barrier.newLayout = newVkLayout;
	barrier.image = image;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	barrier.subresourceRange.aspectMask = chooseAspectMask(format, newLayout);
	
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;
	getAccessFlags(layout, newLayout, barrier.srcAccessMask, barrier.dstAccessMask);

	vkCmdPipelineBarrier(cmd.getHandle(), toVkPipelineStage(srcStage), toVkPipelineStage(dstStage), 0, 0, nullptr, 0, nullptr, 1, &barrier);

	layout = newLayout;
}

void VulkanImage::createImage(ImageExtent2D extent, ImageUsageFlags usage) {

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = extent.width;
	imageInfo.extent.height = extent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = toVkFormat(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = toVkImageUsageFlags(usage);
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create an image!");
	}
}

void VulkanImage::allocateMemory() {
	VkDevice vkDevice = device.getDevice();

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(vkDevice, image, &memReq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = device.findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate memory to an image!");
	}

	vkBindImageMemory(vkDevice, image, memory, 0);
}

void VulkanImage::createImageView(ImageUsageFlags usage) {
	VkImageAspectFlags aspect{};

	aspect = chooseAspectMask(format, layout);

	VkImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = toVkFormat(format);
	viewCreateInfo.subresourceRange.aspectMask = aspect;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device.getDevice(), &viewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create an image's view!");
	}
}