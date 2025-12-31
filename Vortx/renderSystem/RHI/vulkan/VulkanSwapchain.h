#pragma once

#include "common/VulkanFwd.h"
#include "renderSystem/RHI/common/ImageTypes.h"

#include <vector>

class VulkanDevice;

class VulkanSwapchain {
public:
	VulkanSwapchain(VulkanDevice* device, uint32_t width, uint32_t height);
	~VulkanSwapchain();

	ImageFormat getFormat() const { return imageFormat; }
	ImageExtent2D getExtent() const { return extent; }
	uint32_t getImageCount() const { return static_cast<uint32_t>(images.size()); }

	uint32_t accquireNextImage(VkSemaphore semaphore);
	void present(uint32_t imageIndex, VkSemaphore waitSemaphore);

	VkImageView getImageView(uint32_t index) const { return imageViews[index]; }

private:
	void createSwapchain(uint32_t width, uint32_t height);
	void createImageView();

private:
	VulkanDevice* device;

	VkSwapchainKHR swapchain = nullptr;
	ImageFormat imageFormat;
	ImageExtent2D extent;

	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
};