#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <memory>

#include "core/dataDef/Vertex.h"
#include "commProtocols/threadCommProtocol.h"

#include "FastNoiseLite.h"

struct VoxelData {
	std::string name;
	std::string ColorMap;
	std::string NormalMap;
	int index;
};

struct TextureAtlas {
	int atlasWidth;
	int atlasHeight;
	int tileSize;

	std::unordered_map<uint8_t, glm::vec4> uvRanges;

	TextureData ColorData;
	TextureData NormalData;
};

struct genratedChunk {
	glm::ivec3 pos;
	std::unique_ptr<Chunk> chunk;
};

struct MeshJob {
	glm::ivec3 pos;
	MeshData mesh;
};

class World {
public:
	World();
	~World();

	int getChunkCount();
	void reqProximityChunks(const glm::vec3& pos);
	void captureGenratedChunks();
	void updateTerrainConstants();
	void clearLoadedChunks();

	bool chunkShouldExist(const glm::ivec3& pos);
	Chunk* findChunk(const glm::ivec3& pos);

	void updateChunkMesh(const glm::ivec3& pos);

	int getSurfaceZ(glm::vec3 pos);
	void setBlock(int x, int y, int z, int blockType);

	float terrainHeight = 50.0f;
	float terrainScale = 0.01f;

	glm::ivec3 playerChunk = { 0,0,0 };
	int renderDistance = 16;

private:
	FastNoiseLite heightMap;

	TextureAtlas buildTextureAtlas(std::vector<VoxelData>& inputBlocks, int tileSize);

	int getTerrainHeight(int x, int z);
	glm::ivec2 getChunkCoordinates(glm::vec3 pos);

	void GreedyMesher(const Chunk& chunk, std::vector<Vertex>& verts, std::vector<uint32_t>& indices);
	void Mesher(const Chunk& chunk, std::vector<Vertex>& verts, std::vector<uint32_t>& indices);

	std::unique_ptr<Chunk> generateChunk(const glm::ivec3& pos);

	std::atomic<bool> chunkBuilderActive;
	ThreadSafeQueue<glm::ivec3> reqChunks;

	std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> chunks;
	std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> stagingChunks;
	std::mutex chunkMutex;
	std::mutex stagingMutex;

	std::thread ChunkGenerator;
	void chunkBuilderLoop();
	ThreadSafeQueue<glm::ivec3> generatedQueue;

	bool neighborsReady_local(World* world, const glm::ivec3& pos);
	std::thread ChunkMesher;
	void chunkMesherLoop();
	ThreadSafeQueue<MeshJob> meshedChunks;

	void requestChunk(const glm::ivec3& pos);
};