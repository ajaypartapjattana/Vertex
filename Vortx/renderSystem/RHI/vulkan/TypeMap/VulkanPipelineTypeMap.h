#pragma once

#include "renderSystem/RHI/vulkan/Common/VulkanCommon.h"
#include "renderSystem/RHI/common/PipelineTypes.h"

#include "VulkanDescriptorTypeMap.h"

VkShaderStageFlagBits toVkShaderStageFlagBits(ShaderStageBit stage) {
	switch(stage) {
	case ShaderStageBit::VertexBit: return VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderStageBit::GeometryBit: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderStageBit::FragmentBit: return VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderStageBit::ComputeBit: return VK_SHADER_STAGE_COMPUTE_BIT;
	default:
		throw std::runtime_error("invalid shader stage for pipeline stage!");
	}
}

VkFormat toVkFormat(VertexFormat format) {
	switch (format) {
	case VertexFormat::Float2: return VK_FORMAT_R32G32_SFLOAT;
	case VertexFormat::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
	case VertexFormat::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
	case VertexFormat::Uint4: return VK_FORMAT_R32G32B32A32_SINT;
	default: return VK_FORMAT_R32G32B32_SFLOAT;
	}
}

VkVertexInputBindingDescription toVkVertexInputBindingDescription(const VertexBindingDesc& desc) {
	VkVertexInputBindingDescription b{};
	b.binding = desc.binding;
	b.stride = desc.stride;
	b.inputRate = desc.perinstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

	return b;
}

VkVertexInputAttributeDescription toVkVertexInputAttributeDescription(const VertexAttributeDesc& desc, uint32_t binding) {
	VkVertexInputAttributeDescription a{};
	a.binding = binding;
	a.location = desc.location;
	a.format = toVkFormat(desc.format);
	a.offset = desc.offset;
}

VkSampleCountFlagBits toVkSampleCountFlagBits(RasterSamples samples) {
	switch (samples) {
	case RasterSamples::Raster_Samples_1: return VK_SAMPLE_COUNT_1_BIT;
	case RasterSamples::Raster_Samples_2: return VK_SAMPLE_COUNT_2_BIT;
	case RasterSamples::Raster_Samples_4: return VK_SAMPLE_COUNT_4_BIT;
	case RasterSamples::Raster_Samples_8: return VK_SAMPLE_COUNT_8_BIT;
	case RasterSamples::Raster_Samples_16: return VK_SAMPLE_COUNT_16_BIT;
	case RasterSamples::Raster_Samples_32: return VK_SAMPLE_COUNT_32_BIT;
	case RasterSamples::Raster_Samples_64: return VK_SAMPLE_COUNT_64_BIT;
	}
}