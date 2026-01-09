#pragma once

#include "Signboard/RHI/common/VertexTypes.h"

#include <cstdint>

struct MeshDesc {
	const void* p_vertexData;
	size_t vertexSize;
	size_t vertexCount;

	const uint32_t* p_indexData;
	uint32_t indexCount;
	IndexType indexType;
};

struct MeshHandle {
	uint32_t index;
	uint32_t generation;
};
constexpr MeshHandle INVALID_MESH{ UINT32_MAX, UINT32_MAX };