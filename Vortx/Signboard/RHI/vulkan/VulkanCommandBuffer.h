#pragma once

#include "Common/VulkanFwd.h"

class VulkanDevice;
class VulkanCommandPool;
class VulkanBuffer;

class VulkanCommandBuffer {
public:
	VulkanCommandBuffer(VulkanDevice& device, VulkanCommandPool& commandPool);
	~VulkanCommandBuffer();

	void begin();
	void end();

	void submitAndWait();

	void bindVertexBuffer(const VulkanBuffer& buffer);
	void bindIndexBuffer(const VulkanBuffer& buffer);
	void drawIndexed(uint32_t indexCount);

	VkCommandBuffer getHandle() const { return commandBuffer; }

private:
	VulkanDevice& device;
	VulkanCommandPool& commandPool;

	VkCommandBuffer commandBuffer = nullptr;
	VkFence fence = nullptr;
};