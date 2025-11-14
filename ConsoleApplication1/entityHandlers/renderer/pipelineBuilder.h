#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include <iostream>

class PipelineBuilder {
public:
	VkPipelineShaderStageCreateInfo shaderStages[2];
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	VkPipelineViewportStateCreateInfo viewportState{};
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	VkPipelineMultisampleStateCreateInfo multisampling{};
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	VkPipelineColorBlendStateCreateInfo colorBlending{};

	VkViewport viewport{};
	VkRect2D scissor{};

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};

	std::vector<VkDynamicState> dynamicStates;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	VkVertexInputBindingDescription bindingDescription{};

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	PipelineBuilder();

	VkPipeline buildPipeline(VkDevice device);

	void setDefaultInputAssembly();
	void setDefaultRasterizer(VkPolygonMode mode);
	void setDefaultColorBlend();
	void setDefaultDepthStencil(bool enableDepthTest);
};