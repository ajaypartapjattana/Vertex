#include "VulkanSwapchain.h"

#include "TypeMap/VulkanImageTypeMap.h"
#include "TypeMap/VulkanSwapchainTypeMap.h"

#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanSemaphore.h"

VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, uint32_t width, uint32_t height)
	: device(device)
{
	createSwapchain(width, height);
}

VulkanSwapchain::~VulkanSwapchain() {
	VkDevice vkDevice = device.getDevice();

	if (swapchain) {
		vkDestroySwapchainKHR(vkDevice, swapchain, nullptr);
	}
}

void VulkanSwapchain::createSwapchain(uint32_t width, uint32_t height) {
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(), device.getSurface(), &caps);

	extent = { width, height };

	uint32_t imageCount = caps.minImageCount + 1;
	if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
		imageCount = caps.maxImageCount;
	}

	imageFormat = ImageFormat::BGRA8;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = device.getSurface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = toVkFormat(imageFormat);
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent.width = extent.width;
	createInfo.imageExtent.height = extent.height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = caps.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(device.getDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swapchain!");
	}

	uint32_t count = 0;
	vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &count, nullptr);

	std::vector<VkImage> vkImages(count);
	vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &count, vkImages.data());

	for (const VkImage image : vkImages) {
		images.emplace_back(VulkanImage(image, imageFormat, ImageLayout::Present));
	}
}

SwapchainImageAcquire VulkanSwapchain::accquireNextImage(VulkanSemaphore semaphore) {
	uint32_t index;
	VkResult result = vkAcquireNextImageKHR(device.getDevice(), swapchain, UINT64_MAX, semaphore.get(), nullptr, &index);
	
	return SwapchainImageAcquire{toSwapchainAcquireResult(result), index};
}

void VulkanSwapchain::present(uint32_t imageIndex, VulkanSemaphore waitSemaphore) {
	VkSemaphore semaphore = waitSemaphore.get();

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);
}