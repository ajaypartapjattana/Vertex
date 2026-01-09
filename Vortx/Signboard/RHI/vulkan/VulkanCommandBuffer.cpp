#include "VulkanCommandBuffer.h"

#include "Common/VulkanCommon.h"

#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanBuffer.h"


VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, VulkanCommandPool& commandPool)
	: device(device), commandPool(commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool.getHandle();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffer!");
	}

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a fence!");
	}
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
	VkDevice vkDevice = device.getDevice();

	if (fence) {
		vkDestroyFence(vkDevice, fence, nullptr);
	}

	if (commandBuffer) {
		vkFreeCommandBuffers(vkDevice, commandPool.getHandle(), 1, &commandBuffer);
	}
}

void VulkanCommandBuffer::begin() {
	vkWaitForFences(device.getDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device.getDevice(), 1, &fence);

	vkResetCommandBuffer(commandBuffer, 0);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin command buffer!");
	}
}

void VulkanCommandBuffer::end() {
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to end command buffer!");
	}
}

void VulkanCommandBuffer::submitAndWait() {
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit command buffer");
	}

	vkWaitForFences(device.getDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
}

void VulkanCommandBuffer::bindVertexBuffer(const VulkanBuffer& buffer) {
	VkBuffer buffers[] = { buffer.getHandle() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void VulkanCommandBuffer::bindIndexBuffer(const VulkanBuffer& buffer){
	vkCmdBindIndexBuffer(commandBuffer, buffer.getHandle(), 0, VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount){
	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}