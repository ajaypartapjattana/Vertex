#include "VulkanPipeline.h"

#include "TypeMap/VulkanPipelineTypeMap.h"
#include "VulkanDevice.h"
#include "VulkanPipelineCache.h"
#include "VulkanPipelineLayout.h"
#include "VulkanRenderPass.h"

#include <fstream>

struct VulkanPipeline::Impl {
	std::vector<VkVertexInputAttributeDescription> vertexAttributes;
	std::vector<VkVertexInputBindingDescription> vertexBindings;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	VkPipelineRasterizationStateCreateInfo raster{};
	VkPipelineDepthStencilStateCreateInfo depth{};
	VkPipelineColorBlendAttachmentState blend{};
};

VulkanPipeline::VulkanPipeline(VulkanDevice& device, VulkanPipelineCache& cache)
	: device(device), cache(cache), impl(new Impl{}) { }

VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept
	: device(other.device), cache(other.cache), pipeline(other.pipeline), built(other.built), impl(other.impl)
{
	other.pipeline = VK_NULL_HANDLE;
	other.impl = nullptr;
}

VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& other) noexcept {
	if (this != &other) {
		pipeline = other.pipeline;
		impl = other.impl;
		built = other.built;

		other.pipeline = VK_NULL_HANDLE;
		other.impl = nullptr;
	}
	return *this;
}

VulkanPipeline::~VulkanPipeline() {
	if (pipeline)
		vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
	delete impl;
}

void VulkanPipeline::build(const VulkanRenderPass& renderPass, const VulkanPipelineLayout& layout, const PipelineDesc& desc) {
	if (desc.shaders.empty())
		throw std::runtime_error("pipeline must have atleast one shader!");

	if (desc.type == PipelineType::Compute)
		if (desc.shaders.size() != 1 || desc.shaders[0].stage != ShaderStageBit::ComputeBit)
			throw std::runtime_error("compute pipeline must have exactly one compute shader!");

	if (built)
		throw std::runtime_error("pipeline has already been built!");

	std::vector<VkShaderModule> shaderModules;

	if (desc.type == PipelineType::Graphics) {
		for (const VertexBindingDesc& binding : desc.vertexLayout.bindings)
			impl->vertexBindings.push_back(toVkVertexInputBindingDescription(binding));

		for (const VertexAttributeDesc& attribute : desc.vertexLayout.attributes)
			impl->vertexAttributes.push_back(toVkVertexInputAttributeDescription(attribute, 0));

		for (const ShaderDesc& shader : desc.shaders) {
			auto spirv = readSPIRV(shader.path);
			VkShaderModule module = createShaderModule(device.getDevice(), spirv);

			shaderModules.push_back(module);

			VkPipelineShaderStageCreateInfo stage{};
			stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage.stage = toVkShaderStageFlagBits(shader.stage);
			stage.module = module;
			stage.pName = "main";

			impl->shaderStages.push_back(stage);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(impl->vertexBindings.size());
		vertexInputInfo.pVertexBindingDescriptions = impl->vertexBindings.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(impl->vertexAttributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = impl->vertexAttributes.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.rasterizationSamples = toVkSampleCountFlagBits(desc.samples);
		multisampling.minSampleShading = 1.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = desc.raster.depthTest ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = desc.raster.depthWrite ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		if (desc.blend.enable) {
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		}
		else {
			colorBlendAttachment.blendEnable = VK_FALSE;
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkGraphicsPipelineCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		info.stageCount = static_cast<uint32_t>(impl->shaderStages.size());
		info.pStages = impl->shaderStages.data();

		info.layout = layout.getHandle();
		info.renderPass = renderPass.getHandle();
		info.subpass = 0;

		info.pVertexInputState = &vertexInputInfo;
		info.pInputAssemblyState = &inputAssembly;
		info.pViewportState = &viewportState;
		info.pRasterizationState = &rasterizer;
		info.pMultisampleState = &multisampling;
		info.pDepthStencilState = &depthStencil;
		info.pColorBlendState = &colorBlending;
		info.pDynamicState = &dynamicState;

		if (vkCreateGraphicsPipelines(device.getDevice(), cache.getHandle(), 1, &info, nullptr, &pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipelien!");
		}
	}

	if (desc.type == PipelineType::Compute) {
		VkComputePipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		createInfo.stage = impl->shaderStages[0];
		createInfo.layout = layout.getHandle();

		if (vkCreateComputePipelines(device.getDevice(), cache.getHandle(), 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline!");
		}
	}

	for (VkShaderModule module : shaderModules) {
		vkDestroyShaderModule(device.getDevice(), module, nullptr);
	}

	built = true;
}

VkShaderModule createShaderModule(VkDevice device ,const std::vector<uint32_t>& spirv) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spirv.size() * sizeof(uint32_t);
	createInfo.pCode = spirv.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}

std::vector<uint32_t> readSPIRV(const std::string& path) {
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!" + path);
	}

	size_t size = static_cast<size_t>(file.tellg());
	if (size % 4 != 0)
		throw std::runtime_error("SPIR-V file size is not a mutiple of 4: " + path);

	std::vector<uint32_t> buffer(size / 4);
	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), size);
	file.close();

	return buffer;
}