#pragma once

#include <cstdint>

struct Mesh {
	uint32_t vertexBuffer;
	uint32_t indexBuffer;

	size_t vertexOffset;
	size_t indexOffset;

	uint32_t indexCount;
};