#pragma once

#include "renderSystem/RHI/vulkan/Common/VulkanCommon.h"
#include "renderSystem/RHI/common/DescriptorTypes.h"

VkDescriptorType toVkDescriptorType(DescriptorType type) {
	switch (type) {
	case DescriptorType::CombinedImageSampler:	return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case DescriptorType::UniformBuffer:			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case DescriptorType::SampledImage:			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case DescriptorType::TextureSampler:		return VK_DESCRIPTOR_TYPE_SAMPLER;
	}
}

VkShaderStageFlags toVkShaderStageFlags(ShaderStageFlags stage) {
	struct Map { ShaderStageBit stageBit; VkShaderStageFlagBits vk; };

	static constexpr Map table[] = {
		{ShaderStageBit::VertexBit,		VK_SHADER_STAGE_VERTEX_BIT},
		{ShaderStageBit::GeometryBit,	VK_SHADER_STAGE_GEOMETRY_BIT},
		{ShaderStageBit::FragmentBit,	VK_SHADER_STAGE_FRAGMENT_BIT},
		{ShaderStageBit::ComputeBit,	VK_SHADER_STAGE_COMPUTE_BIT}, 
		{ShaderStageBit::AllBit,		VK_SHADER_STAGE_ALL}
	};

	VkShaderStageFlags flags = 0;
	for (auto& mapping : table)
		if (stage.has(mapping.stageBit))
			flags |= mapping.vk;

	return flags;
}

VkDescriptorBindingFlags toVkDescriptorBindingFlags(DescriptorBindingFlags bindFlags, bool& updateBool) {
	struct Map { DescriptorBindingBit bindingBit; VkDescriptorBindingFlagBits vk; };

	static constexpr Map table[] = {
		{DescriptorBindingBit::PatiallyBound, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
		{DescriptorBindingBit::UpdateAfterBind, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
		{DescriptorBindingBit::VariableCount, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT}
	};

	VkDescriptorBindingFlags flags = 0;
	for (auto& mapping : table)
		if (bindFlags.has(mapping.bindingBit))
			flags |= mapping.vk;

	if (bindFlags.has(DescriptorBindingBit::UpdateAfterBind))
		updateBool = true;

	return flags;
}