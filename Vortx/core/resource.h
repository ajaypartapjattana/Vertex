#pragma once

#include "datadef/Vertex.h"

struct MeshData {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

struct TextureData{
	int width = 0;
	int height = 0;
	int channels = 4;
	std::vector<uint8_t> pixels;
};

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 256;

struct Chunk {
	glm::ivec3 chunkPos{};

	uint8_t voxels[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE] = {};

	MeshData meshData;

	bool dirty = true;
	uint64_t version = 0;

	inline uint8_t get(int x, int y, int z) const {
		if (x < 0 || x >= CHUNK_SIZE ||
			y < 0 || y >= CHUNK_HEIGHT ||
			z < 0 || z >= CHUNK_SIZE)
			return 0;
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
