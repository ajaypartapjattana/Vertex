#pragma once

#include <cstdint>

using PassID = uint32_t;
static constexpr PassID INVALID_PASS = UINT32_MAX;

using MaterialID = uint32_t;
static constexpr MaterialID INVALID_MATERIAL = UINT32_MAX;

using PipelineID = uint32_t;
static const PipelineID INVALID_PIPELINE = UINT32_MAX;

struct descriptorSetLayouts {
	VkDescriptorSetLayout globalLayout;
	VkDescriptorSetLayout objectLayout;
	VkDescriptorSetLayout materialLayout;
};

struct UniformAllocation {
	VkDescriptorSet descriptorSet;

	VkBuffer UBO;
	VkDeviceMemory objectMemory;
	void* mappedPtr;
};