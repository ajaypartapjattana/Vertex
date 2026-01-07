#pragma once

#include "DescriptorTypes.h"
#include "ImageTypes.h"
#include "VertexTypes.h"

#include <string>
#include <vector>

// VulkanPipelineLayout types ---
struct PushConstantRangeDesc {
	ShaderStageFlags stages;
	uint32_t offset;
	uint32_t size;
};

struct VulkanPipelineLayoutDesc {
	std::vector<const DescriptorSetLayoutDesc*> setLayouts;
	std::vector<PushConstantRangeDesc> pushConstants;
};


// VulkanPipeline types ---
enum class PipelineType {
	Graphics,
	Compute
};

enum class RasterSamples {
	Raster_Samples_1,
	Raster_Samples_2,
	Raster_Samples_4,
	Raster_Samples_8,
	Raster_Samples_16,
	Raster_Samples_32,
	Raster_Samples_64
};

struct ShaderDesc {
	ShaderStageBit stage;
	std::string path;
};

struct RasterState {
	bool depthTest = true;
	bool depthWrite = true;
};

struct BlendState {
	bool enable = false;
};

struct PipelineDesc {
	PipelineType type;

	VertexLayoutDesc vertexLayout;

	RasterSamples samples;

	ImageFormat colorFormat;
	ImageFormat depthForamt;

	std::vector<ShaderDesc> shaders;

	RasterState raster;
	BlendState blend;
};