#include "VulkanCommandPool.h"

#include "Common/VulkanCommon.h"
#include "VulkanDevice.h"

VulkanCommandPool::VulkanCommandPool(VulkanDevice* device)
	: device(device)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = device->getGraphicsQueueFamily();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	if (vkCreateCommandPool(device->getDevice(), &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

VulkanCommandPool::~VulkanCommandPool() {
	if (commandPool) {
		vkDestroyCommandPool(device->getDevice(), commandPool, nullptr);
	}
}