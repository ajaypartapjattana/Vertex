#pragma once

#include "base/Flag_type.h"

enum class Filter {
	Linear,
	Nearest
};

enum class SamplerAddressMode {
	Repeat,
	MirroredRepeat,
	ClampEdge
};

struct VulkanSamplerDesc {
	Filter magFilter = Filter::Linear;
	Filter minFilter = Filter::Linear;

	SamplerAddressMode addressU = SamplerAddressMode::Repeat;
	SamplerAddressMode addressV = SamplerAddressMode::Repeat;
	SamplerAddressMode addressW = SamplerAddressMode::Repeat;

	bool anisotropyEnable = true;
	float maxAnisotropy = 16.0f;
};