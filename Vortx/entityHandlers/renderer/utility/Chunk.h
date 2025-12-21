#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "Vertex.h"

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 256;

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

struct MeshJob{
	glm::ivec3 pos;
	Mesh mesh;
};

struct Chunk {
	glm::ivec3 chunkPos{};

	uint8_t voxels[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE] = { 0 };

	Mesh chunkMesh;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
	VkDeviceMemory indexMemory = VK_NULL_HANDLE;

	bool dirty = true;
	bool gpuAllocated = false;

	inline uint8_t get(int x, int y, int z) const {
		if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE) return 0;
		return voxels[x][y][z];
	}
};

struct IVec3Hash {
	std::size_t operator()(const glm::ivec3& v) const noexcept {
		size_t h1 = std::hash<int>()(v.x);
		size_t h2 = std::hash<int>()(v.y);
		size_t h3 = std::hash<int>()(v.z);
		return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
	}
};

struct IVec3Equal {
	bool operator()(const glm::ivec3& a, const glm::ivec3& b) const noexcept {
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};