#include "VulkanRenderPass.h"

#include "TypeMap/VulkanRenderPassTypeMap.h"
#include "VulkanDevice.h"

struct VulkanRenderPass::Impl {
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> colorRefs;
	VkAttachmentReference depthRef{};
	bool hasDepth = false;
};

VulkanRenderPass::VulkanRenderPass(VulkanDevice& device, const RenderPassDesc& desc) 
	: device(device), impl(new Impl{})
{
	for (const auto& att : desc.colorAttachments) {
		VkAttachmentDescription attachment{};
		attachment.format = toVkFormat(att.format);
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = toVkAttachmentLoadOp(att.load);
		attachment.storeOp = toVkAttachmentStoreOp(att.store);
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		impl->attachments.push_back(attachment);

		VkAttachmentReference ref{};
		ref.attachment = uint32_t(impl->attachments.size() - 1);
		ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		impl->colorRefs.push_back(ref);
	}

	if (desc.hasDepth) {
		VkAttachmentDescription attachment{};
		attachment.format = toVkFormat(desc.depthAttachment.format);
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = toVkAttachmentLoadOp(desc.depthAttachment.load);
		attachment.storeOp = toVkAttachmentStoreOp(desc.depthAttachment.store);
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		impl->attachments.push_back(attachment);

		impl->depthRef.attachment = uint32_t(impl->attachments.size() - 1);
		impl->depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		impl->hasDepth = true;
	}

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = uint32_t(impl->colorRefs.size());
	subpass.pColorAttachments = impl->colorRefs.data();
	subpass.pDepthStencilAttachment = impl->hasDepth ? &impl->depthRef : nullptr;

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<uint32_t>(impl->attachments.size());
	createInfo.pAttachments = impl->attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(device.getDevice(), &createInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

VulkanRenderPass::~VulkanRenderPass() {
	if (renderPass)
		vkDestroyRenderPass(device.getDevice(), renderPass, nullptr);
	
	delete impl;
}

