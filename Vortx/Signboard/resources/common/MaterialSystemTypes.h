#pragma once

#include "PipelineSystemTypes.h"
#include "TextureSystemType.h"
#include "SamplerSystemTypes.h"

struct MaterialHandle {
	uint32_t index;
	uint32_t generation;

	bool isValid() const {
		return generation != 0;
	}
};
constexpr MaterialHandle INVALID_MATERIAL{ UINT32_MAX, UINT32_MAX };

struct MaterialDesc {
	PipelineHandle pipeline;
	GPUMaterial material;
};

struct GPUMaterial {
	glm::vec4 baseColor;
	float metallic;
	float roughness;
	uint32_t albedoTex;
	uint32_t normaltex;
};