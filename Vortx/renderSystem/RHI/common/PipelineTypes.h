#pragma once

#include "DescriptorTypes.h"
#include "ImageTypes.h"

#include <string>
#include <vector>

// VulkanPipelineLayout types ---
struct PushConstantRangeDesc {
	ShaderStageFlags stages;
	uint32_t offset;
	uint32_t size;
};

struct VulkanPipelineLayoutDesc {
	std::vector<VulkanDescriptorSetLayoutDesc> setLayouts;
	std::vector<PushConstantRangeDesc> pushConstants;
};


// VulkanPipeline types ---
enum class PipelineType {
	Graphics,
	Compute
};

struct ShaderDesc {
	ShaderStage stage;
	std::string path;
};

struct RasterState {
	bool depthTest = true;
	bool depthWrite = true;
};

struct BlendState {
	bool enable = false;
};

struct VulkanPipelineDesc {
	PipelineType type;
	VulkanPipelineLayoutDesc layout;

	std::vector<ShaderDesc> shaders;

	ImageFormat colorFormat;
	ImageFormat depthForamt;

	RasterState raster;
	BlendState blend;
};