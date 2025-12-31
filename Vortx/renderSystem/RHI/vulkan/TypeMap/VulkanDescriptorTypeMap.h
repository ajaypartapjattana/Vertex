#pragma once

#include "renderSystem/RHI/vulkan/Common/VulkanCommon.h"
#include "renderSystem/RHI/common/DescriptorTypes.h"

VkDescriptorType toVkDescriptorType(DescriptorType type) {
	switch (type) {
	case DescriptorType::CombinedImageSampler:	return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case DescriptorType::UniformBuffer:			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
}

VkShaderStageFlags toVkShaderStageFlags(ShaderStageFlags stage) {
	struct Map { ShaderStage stageBit; VkShaderStageFlagBits vk; };

	static constexpr Map table[] = {
		{ShaderStage::VertexBit,	VK_SHADER_STAGE_VERTEX_BIT},
		{ShaderStage::GeometryBit,	VK_SHADER_STAGE_GEOMETRY_BIT},
		{ShaderStage::FragmentBit,	VK_SHADER_STAGE_FRAGMENT_BIT},
		{ShaderStage::AllBit,		VK_SHADER_STAGE_ALL}
	};

	VkShaderStageFlags flags = 0;
	for (auto& mapping : table)
		if (stage.has(mapping.stageBit))
			flags |= mapping.vk;

	return flags;
}