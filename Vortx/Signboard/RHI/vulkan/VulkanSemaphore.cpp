#include "VulkanSemaphore.h"

#include "Common/VulkanCommon.h"
#include "VulkanDevice.h"

VulkanSemaphore::VulkanSemaphore(VulkanDevice& device)
	: device(device)
{
	VkSemaphoreCreateInfo createinfo{};
	createinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createinfo.flags = 0;

	vkCreateSemaphore(device.getDevice(), &createinfo, nullptr, &semaphore);
}

VulkanSemaphore::~VulkanSemaphore() {
	if (semaphore)
		vkDestroySemaphore(device.getDevice(), semaphore, nullptr);
}