#pragma once

#include "renderSystem/RHI/vulkan/Common/VulkanCommon.h"
#include "renderSystem/RHI/common/ImageTypes.h"

VkFormat toVkFormat(ImageFormat format) {
	switch (format) {
	case ImageFormat::RGBA8:					return VK_FORMAT_R8G8B8A8_UNORM;
	case ImageFormat::BGRA8:					return VK_FORMAT_B8G8R8A8_UNORM;
	case ImageFormat::RGBA16F:					return VK_FORMAT_R16G16B16A16_SFLOAT;
	case ImageFormat::Depth24Stencil8:			return VK_FORMAT_D24_UNORM_S8_UINT;
	case ImageFormat::Depth32F:					return VK_FORMAT_D32_SFLOAT;
	default:									return VK_FORMAT_UNDEFINED;
	}
}

inline bool hasDepthComponent(ImageFormat format) {
	switch (format) {
	case ImageFormat::Depth24Stencil8:
	case ImageFormat::Depth32F:
		return true;
	default:
		return false;
	}
}

inline bool hasStencilComponent(ImageFormat format) {
	switch (format) {
	case ImageFormat::Depth24Stencil8:
		return true;
	default:
		return false;
	}
}

VkImageLayout toVkImageLayout(ImageLayout layout) {
	switch (layout) {
	case ImageLayout::Undefined:				return VK_IMAGE_LAYOUT_UNDEFINED;
	case ImageLayout::TransferDst:				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	case ImageLayout::TransferSrc:				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case ImageLayout::ShaderReadOnly:			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ImageLayout::ColorAttachment:			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ImageLayout::DepthStencilAttachment:	return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case ImageLayout::Present:					return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	default:									return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}

VkImageUsageFlags toVkImageUsageFlags(ImageUsageFlags usage) {
	struct Map { ImageUsage usageBit; VkImageUsageFlags vk; };

	static constexpr Map table[] = {
		{ImageUsage::ColorAttachment,			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT},
		{ImageUsage::DepthAttachment,			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT},
		{ImageUsage::Sampled,					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT}
	};

	VkImageUsageFlags flags = 0;
	for (auto& mapping : table)
		if (usage.has(mapping.usageBit))
			flags |= mapping.vk;

	return flags;
}

VkPipelineStageFlags toVkPipelineStage(PipelineStageFlags stage) {
	struct Map { PipelineStage stageBit; VkPipelineStageFlagBits vk; };

	static constexpr Map table[] = {
		{PipelineStage::TopOfPipe,				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
		{PipelineStage::Transfer,				VK_PIPELINE_STAGE_TRANSFER_BIT},
		{PipelineStage::VertexInput,			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT},
		{PipelineStage::VertexShader,			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT},
		{PipelineStage::FragmentShader,			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
		{PipelineStage::EarlyDepthTest,			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT},
		{PipelineStage::LateDepthTest,			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT},
		{PipelineStage::ColorAttachmentOutput,	VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
		{PipelineStage::BottomOfPipe,			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}
	};

	VkPipelineStageFlags flags = 0;
	for (auto& mapping : table)
		if (stage.has(mapping.stageBit))
			flags |= mapping.vk;

	return flags;
}