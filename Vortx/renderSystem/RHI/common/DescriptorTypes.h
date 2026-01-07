#pragma once

#include "base/Flag_type.h"

enum class DescriptorType {
	CombinedImageSampler,
	UniformBuffer,
	StorageBuffer,
	SampledImage,
	TextureSampler
};

enum class ShaderStageBit {
	VertexBit   = 1 << 0,
	GeometryBit = 1 << 1,
	FragmentBit = 1 << 2,
	ComputeBit	= 1 << 3,
	AllBit		= 1 << 4
};

using ShaderStageFlags = Flags<ShaderStageBit>;

enum class DescriptorBindingBit : uint32_t {
	None = 0,
	PatiallyBound = 1 << 0,
	UpdateAfterBind = 1 << 1,
	VariableCount = 1 << 2
};

using DescriptorBindingFlags = Flags<DescriptorBindingBit>;

// VulkanDescriptorLayout types	---
struct DescriptorBindingDesc {
	uint32_t binding;
	DescriptorType type;
	uint32_t count;
	ShaderStageFlags stages;
	DescriptorBindingFlags flags = DescriptorBindingBit::None;

};

struct DescriptorSetLayoutDesc {
	std::vector<DescriptorBindingDesc> bindings;
};


// VulkanDescriptorPool types	---
struct DescriptorPoolSizeDesc {
	DescriptorType type;
	uint32_t count;
};

struct DescriptorPoolDesc {
	uint32_t maxSets = 0;
	std::vector<DescriptorPoolSizeDesc> poolSizes;
};