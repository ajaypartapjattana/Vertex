#pragma once

#include "Common/VulkanFwd.h"
#include "renderSystem/RHI/common/RenderPassTypes.h"

class VulkanDevice;

class VulkanRenderPass {
public:
	VulkanRenderPass(VulkanDevice& device, const RenderPassDesc& desc);
	
	VulkanRenderPass(const VulkanRenderPass&) = delete;
	VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;
	
	VulkanRenderPass(VulkanRenderPass&&) noexcept;
	VulkanRenderPass& operator=(VulkanRenderPass&&) noexcept;

	~VulkanRenderPass();

	VkRenderPass getHandle() const { return renderPass; }

private:
	struct Impl;
	Impl* impl;

	VulkanDevice& device;
	
	VkRenderPass renderPass = nullptr;

};