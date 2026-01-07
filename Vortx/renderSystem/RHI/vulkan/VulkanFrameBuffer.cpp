#include "VulkanFrameBuffer.h"

#include "Common/VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanImage.h"

struct VulkanFrameBuffer::Impl {
	std::vector<VkImageView> attachments;
};

VulkanFrameBuffer::VulkanFrameBuffer(VulkanDevice& device, VulkanRenderPass& renderpass, const FrameBufferDesc& desc)
	: device(device), renderPass(renderpass), impl(new Impl{}), width(desc.width), height(desc.height)
{
	for (const VulkanImage* image : desc.colorAttachments) {
		impl->attachments.push_back(image->getView());
	}

	if (desc.depthAttachment)
		impl->attachments.push_back(desc.depthAttachment->getView());

	VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	createInfo.attachmentCount = static_cast<uint32_t>(impl->attachments.size());
	createInfo.pAttachments = impl->attachments.data();
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = desc.layers;

	if (vkCreateFramebuffer(device.getDevice(), &createInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffers!");
	}
}

VulkanFrameBuffer::VulkanFrameBuffer(VulkanFrameBuffer&& other)
	: device(other.device), renderPass(other.renderPass), impl(other.impl), frameBuffer(other.frameBuffer), width(other.width), height(other.height)
{
	other.impl = nullptr;
	other.frameBuffer = VK_NULL_HANDLE;
}

VulkanFrameBuffer& VulkanFrameBuffer::operator=(VulkanFrameBuffer&& other) {
	if (this == &other)
		return *this;

	if (frameBuffer)
		vkDestroyFramebuffer(device.getDevice(), frameBuffer, nullptr);

	delete impl;

	impl = other.impl;
	frameBuffer = other.frameBuffer;
	width = other.width;
	height = other.height;

	other.impl = nullptr;
	other.frameBuffer = VK_NULL_HANDLE;

	return *this;
}

VulkanFrameBuffer::~VulkanFrameBuffer() {
	if (frameBuffer)
		vkDestroyFramebuffer(device.getDevice(), frameBuffer, nullptr);

	delete impl;
}