#pragma once

#include "base/Flag_type.h"

enum class DescriptorType {
	CombinedImageSampler,
	UniformBuffer
};

enum class ShaderStage {
	VertexBit   = 1 << 0,
	GeometryBit = 1 << 1,
	FragmentBit = 1 << 2,
	AllBit		= 1 << 3
};

using ShaderStageFlags = Flags<ShaderStage>;


// VulkanDescriptorLayout types	---
struct DescriptorBindingDesc {
	uint32_t binding;
	DescriptorType type;
	uint32_t count;
	ShaderStageFlags stages;
};

struct VulkanDescriptorSetLayoutDesc {
	std::vector<DescriptorBindingDesc> bindings;
};


// VulkanDescriptorPool types	---
struct DescriptorPoolSizeDesc {
	DescriptorType type;
	uint32_t count;
};

struct VulkanDescriptorPoolDesc {
	uint32_t maxSets = 0;
	std::vector<DescriptorPoolSizeDesc> poolSizes;
};