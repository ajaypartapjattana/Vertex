#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/ImageTypes.h"

class VulkanDevice;
class VulkanCommandBuffer;
class VulkanSampler;
class VulkanBuffer;

class VulkanImage {
public:
	VulkanImage(VulkanDevice* device, const VulkanImageDesc& desc, VulkanSampler* sampler = nullptr);

	VulkanImage(const VulkanImage&) = delete;
	VulkanImage& operator=(const VulkanImage&) = delete;

	VulkanImage(VulkanImage&& other) noexcept;
	VulkanImage& operator=(VulkanImage&& other) noexcept;

	~VulkanImage();
	void destroy();

	VkImage getImage() const { return image; }
	VkImageView getView() const { return imageView; }
	VkSampler getSampler() const;

	ImageFormat getFormat() const { return format; }
	ImageLayout getLayout() const { return layout; }

	void copyFromBuffer(VulkanCommandBuffer& cmd, const VulkanBuffer& src);

	void assignSampler(VulkanSampler* sampler);
	void transitionLayout(VulkanCommandBuffer& commandBuffer, ImageLayout newLayout, PipelineStageFlags srcStage, PipelineStageFlags dstStage);

private:
	void createImage(ImageExtent2D extent, ImageUsageFlags usage);
	void createImageView(ImageUsageFlags usage);
	void allocateMemory();

private:
	VulkanDevice* device = nullptr;

	VulkanSampler* sampler;

	ImageFormat format = ImageFormat::RGBA8;
	ImageLayout layout = ImageLayout::Undefined;
	ImageExtent2D extent;

	VkImage image = nullptr;
	VkDeviceMemory memory = nullptr;
	VkImageView imageView = nullptr;

};