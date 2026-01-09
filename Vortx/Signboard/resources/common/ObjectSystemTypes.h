#pragma once

#include "MeshSystemTypes.h"
#include "MaterialSystemTypes.h"

#include <stdint.h>
#include <glm/glm.hpp>

struct ObjectHandle {
	uint32_t index;
	uint32_t generation;
};
static constexpr ObjectHandle INVALID_OBJECT{ UINT32_MAX, UINT32_MAX };

struct GPUObject {
	glm::mat4 model;
	uint32_t materialIndex;
	uint32_t meshIndex;
	uint32_t _pad[2];
};

struct ObjectParams {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

struct objectUniformAllocation;

struct RenderObject {
	ObjectHandle id;

	uint32_t meshHandle;
	uint32_t materialID;

	ObjectParams params;
	bool tranformDirty = true;

	std::vector<objectUniformAllocation> perFrameDescriptor;
};