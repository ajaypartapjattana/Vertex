#pragma once

#include "Signboard/RHI/vulkan/Common/VulkanCommon.h"
#include "Signboard/RHI/common/DeviceTypes.h"

VkMemoryPropertyFlags toVkMemoryPropertyFlags(MemoryPropertyFlags property) {
	struct Map { MemoryProperty e; VkMemoryPropertyFlagBits vk; };

	static constexpr Map table[] = {
		{MemoryProperty::HostCached,	VK_MEMORY_PROPERTY_HOST_CACHED_BIT},
		{MemoryProperty::HostVisible,	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT},
		{MemoryProperty::HostCoherent,	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
		{MemoryProperty::DeviceLocal,	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}
	};

	VkBufferUsageFlags flags = 0;
	for (auto& m : table)
		if (property.has(m.e))
			flags |= m.vk;

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