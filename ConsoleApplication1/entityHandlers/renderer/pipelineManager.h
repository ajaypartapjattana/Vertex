#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <fstream>
#include <cstring>

using PipelineHandle = uint32_t;
static const PipelineHandle INVALID_PIPELINE = UINT32_MAX;

struct PipelineVertexInput {
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;
};

struct PipelineDescription {
	std::string vertShaderPath;
	std::string fragShaderPath;

	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	PipelineVertexInput vertexInput;

	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	bool depthTest = true;
	bool depthWrite = true;
	bool blending = false;

	VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
	VkSampleCountFlagBits rasterSamples = VK_SAMPLE_COUNT_1_BIT;

	uint32_t subpass = 0;
};

struct Pipeline {
	VkPipeline pipeline = VK_NULL_HANDLE;
	PipelineDescription description;
};

class PipelineManager {
public:
	void PipelineManager_init(VkDevice device);
	~PipelineManager();

	PipelineHandle createPipleine(const PipelineDescription& desc);
	Pipeline& get(PipelineHandle handle);
	void destroyAll();
private:
	std::vector<Pipeline> pipelines;

	VkDevice device = VK_NULL_HANDLE;
	VkPipelineCache pipelineCache;

	//helpers:
	std::vector<char> readFile(const std::string& path);
	VkShaderModule createShaderModule(const std::vector<char>& code);
};

