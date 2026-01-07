#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/frameBufferTypes.h"

class VulkanDevice;
class VulkanRenderPass;

class VulkanFrameBuffer {
public:
	VulkanFrameBuffer(VulkanDevice& device, VulkanRenderPass& renderPass, const FrameBufferDesc& desc);
	
	VulkanFrameBuffer(const VulkanFrameBuffer&) = delete;
	VulkanFrameBuffer& operator=(const VulkanFrameBuffer&) = delete;

	VulkanFrameBuffer(VulkanFrameBuffer&&) noexcept;
	VulkanFrameBuffer& operator=(VulkanFrameBuffer&&) noexcept;

	~VulkanFrameBuffer();

	VkFramebuffer getHandle() const { return frameBuffer; }

	uint32_t getWidth()  const { return width; }
	uint32_t getHeight() const { return height; }

private:
	struct Impl;
	Impl* impl = nullptr;

	VulkanDevice& device;
	VulkanRenderPass& renderPass;

	VkFramebuffer frameBuffer = nullptr;

	uint32_t width = 0;
	uint32_t height = 0;

};