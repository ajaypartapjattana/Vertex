#include "world.h"

#include <stdexcept>
#include <iostream>
#include <random>

#define _CRT_SECURE_NO_WARNINGS


#pragma warning(push)
#pragma warning(disable: 4996)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "renderer/utility/stb_image_write.h"
#pragma warning(pop)

World::World() 
	: 
	chunkBuilderActive(true),
	ChunkGenerator(&World::chunkBuilderLoop, this),
	ChunkMesher(&World::chunkMesherLoop, this)
{
	heightMap.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	heightMap.SetFrequency(terrainScale);

	VoxelData grass{};
	grass.name = "grass";
	grass.ColorMap = "block_textures/grass_256.png";
	grass.NormalMap = "block_textures/grass_256_Normal.png";
	grass.index = 2;

	VoxelData stone{};
	stone.name = "stone";
	stone.ColorMap = "block_textures/stone_256.png";
	stone.NormalMap = "block_textures/stone_256_Normal.png";
	stone.index = 1;

	VoxelData dirt{};
	dirt.name = "dirt";
	dirt.ColorMap = "block_textures/dirt_256.png";
	dirt.NormalMap = "block_textures/dirt_256_Normal.png";
	dirt.index = 3;

	VoxelData netherack{};
	netherack.name = "netherack";
	netherack.ColorMap = "block_textures/netherack_256.png";
	netherack.NormalMap = "block_textures/netherack_256_Normal.png";
	netherack.index = 4;

	std::vector<VoxelData> inBlocks = { grass , stone, dirt, netherack };
	atlas = buildTextureAtlas(inBlocks, 256);
}

World::~World() {
	chunkBuilderActive = false;
	if (ChunkGenerator.joinable()) ChunkGenerator.join();
	if (ChunkMesher.joinable()) ChunkMesher.join();
}

std::unique_ptr<Chunk> World::generateChunk(const glm::ivec3& pos) {
	if (chunks.find(pos) != chunks.end()) return nullptr;

	//std::cout << "genrating chunk at : [" << pos.x << "," << pos.z << "]" << std::endl;

	auto chunk = std::make_unique<Chunk>();
	chunk->chunkPos = pos;
	chunk->meshData.vertices.clear();
	chunk->meshData.indices.clear();

	int baseX = pos.x * CHUNK_SIZE;
	int baseZ = pos.z * CHUNK_SIZE;

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++) {
			int worldX = baseX + x;
			int worldZ = baseZ + z;

			int terrainHeight = getTerrainHeight(worldX, worldZ);

			for (int y = 0; y < CHUNK_HEIGHT; y++) {
				if (y < terrainHeight - 3) chunk->voxels[x][y][z] = 1;
				else if (y < terrainHeight) chunk->voxels[x][y][z] = 3;
				else if (y == terrainHeight) chunk->voxels[x][y][z] = 2;
				else chunk->voxels[x][y][z] = 0;
			}
		}

	//GreedyMesher(*chunk, chunk->chunkMesh.vertices, chunk->chunkMesh.indices);
	//Mesher(*chunk, chunk->chunkMesh.vertices, chunk->chunkMesh.indices);

	chunk->dirty = true;
	return chunk;
}

void World::Mesher(const Chunk& chunk, std::vector<Vertex>& verts, std::vector<uint32_t>& indices) {
	glm::ivec3 pos = chunk.chunkPos;

	verts.clear();
	indices.clear();

	const glm::ivec3 faceNormals[6] = {
		{ 1, 0, 0 },
		{-1, 0, 0 },
		{ 0, 1, 0 },
		{ 0,-1, 0 },
		{ 0, 0, 1 },
		{ 0, 0,-1 }
	};

	const glm::vec4 faceTangents[6] = {
		{ 0, 0, 1,  1 },
		{ 0, 0, 1, -1 },
		{ 1, 0, 0,  1 },
		{ 1, 0, 0, -1 },
		{ 1, 0, 0,  1 },
		{ 1, 0, 0, -1 }
	};

	const glm::vec3 faceVertices[6][4] = {
		{ {1,0,0}, {1,1,0}, {1,1,1}, {1,0,1} },
		{ {0,0,1}, {0,1,1}, {0,1,0}, {0,0,0} },
		{ {0,1,1}, {1,1,1}, {1,1,0}, {0,1,0} },
		{ {0,0,0}, {1,0,0}, {1,0,1}, {0,0,1} },
		{ {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1} },
		{ {1,0,0}, {0,0,0}, {0,1,0}, {1,1,0} }
	};

	auto isSolid = [&](int WorldX, int WorldY, int WorldZ) -> bool {
		glm::ivec3 cpos((int)std::floor(WorldX / (float)CHUNK_SIZE), 0, (int)std::floor(WorldZ / (float)CHUNK_SIZE));
		Chunk* c = findChunk(cpos);
		if (!c) return 0;
		int lx = WorldX - cpos.x * CHUNK_SIZE;
		int ly = WorldY;
		int lz = WorldZ - cpos.z * CHUNK_SIZE;
		return (c->get(lx, ly, lz) != 0);
	};

	auto getUVForBlock = [&](uint8_t block) -> glm::vec4 {
		auto it = atlas.uvRanges.find(block);
		if (it == atlas.uvRanges.end()) {
			static glm::vec4 defaultUV(0.0f, 0.0f, 1.0f, 1.0f);
			return defaultUV;
		}
		return it->second;
	};

	int baseWX = pos.x * CHUNK_SIZE;
	int baseWZ = pos.z * CHUNK_SIZE;

	for (int x = 0; x < CHUNK_SIZE; x++){
		for (int y = 0; y < CHUNK_HEIGHT; y++){
			for (int z = 0; z < CHUNK_SIZE; z++){
				uint8_t block = chunk.get(x, y, z);
				if (!block) continue;

				glm::vec4 uvRect = getUVForBlock(block);
				glm::vec2 uvFace[4] = {
					{uvRect.x, uvRect.y},
					{uvRect.z, uvRect.y},
					{uvRect.z, uvRect.w},
					{uvRect.x, uvRect.w}
				};

				int WorldX = baseWX + x;
				int WorldY = y;
				int WorldZ = baseWZ + z;

				for (int f = 0; f < 6; f++){
					glm::ivec3 n = faceNormals[f];

					int nx = WorldX + n.x;
					int ny = WorldY + n.y;
					int nz = WorldZ + n.z;

					if (isSolid(nx, ny, nz)) continue;

					uint32_t baseIndex = static_cast<uint32_t>(verts.size());
					for (int v = 0; v < 4; v++){
						Vertex vert;
						vert.pos = glm::vec3(WorldX, WorldY, WorldZ) + faceVertices[f][v];
						vert.normal = glm::vec3(n);
						vert.color = {1.0f,1.0f,1.0f};
						vert.texCoord = uvFace[v];
						vert.tangent = faceTangents[f];
						verts.push_back(vert);
					}
					indices.push_back(baseIndex + 0);
					indices.push_back(baseIndex + 1);
					indices.push_back(baseIndex + 2);
					indices.push_back(baseIndex + 2);
					indices.push_back(baseIndex + 3);
					indices.push_back(baseIndex + 0);
				}
			}
		}
	}
}

TextureAtlas World::buildTextureAtlas(std::vector<VoxelData>& inputBlocks, int tileSize) {
	TextureAtlas result;
	result.tileSize = tileSize;
	int blockCount = static_cast<int>(inputBlocks.size());

	int atlasTilesPerRow = (int)std::ceil(std::sqrt(blockCount));
	int atlasSizePx = atlasTilesPerRow * tileSize;

	result.atlasWidth = result.atlasHeight = atlasSizePx;

	result.ColorData.resize(atlasSizePx * atlasSizePx * 4);
	std::fill(result.ColorData.begin(), result.ColorData.end(), 0);
	result.NormalData.resize(atlasSizePx * atlasSizePx * 4);
	std::fill(result.NormalData.begin(), result.NormalData.end(), 0);

	auto fillData = [&](std::vector<unsigned char>& dst, unsigned char* src, int dstX, int dstY) -> void {
		for (int yy = 0; yy < tileSize; yy++) {
			for (int xx = 0; xx < tileSize; xx++) {
				int srcIndex = (yy * tileSize + xx) * 4;
				int dstIndex = ((dstY + yy) * atlasSizePx + (dstX + xx)) * 4;
				dst[dstIndex + 0] = src[srcIndex + 0];
				dst[dstIndex + 1] = src[srcIndex + 1];
				dst[dstIndex + 2] = src[srcIndex + 2];
				dst[dstIndex + 3] = src[srcIndex + 3];
			}
		}
	};

	int index = 0;
	for (auto& block : inputBlocks) {
		int xTile = index % atlasTilesPerRow;
		int yTile = index / atlasTilesPerRow;

		index++;

		int dstX = xTile * tileSize;
		int dstY = yTile * tileSize;

		int w, h, comp;
		unsigned char* ColorData = stbi_load(block.ColorMap.c_str(), &w, &h, &comp, 4);
		unsigned char* NormalData = stbi_load(block.NormalMap.c_str(), &w, &h, &comp, 4);

		if (!ColorData || !NormalData) {
			std::cerr << "texture data missing block: " << block.ColorMap << "\n";
			continue;
		}
		if (w != tileSize || h != tileSize) {
			std::cerr << "Warning: " << block.ColorMap << " not " << tileSize << "x" << tileSize << "\n";
		}

		fillData(result.ColorData, ColorData, dstX, dstY);
		fillData(result.NormalData, NormalData, dstX, dstY);

		stbi_image_free(ColorData);
		stbi_image_free(NormalData);

		float px = (float)atlasSizePx;
		float inset = 0.5f / px;

		float u0 = (dstX + inset) / px;
		float v0 = (dstY + inset) / px;
		float u1 = (dstX + tileSize - inset) / px;
		float v1 = (dstY + tileSize - inset) / px;

		glm::vec4 uvRange = { u0,v0,u1,v1 };
		result.uvRanges[block.index] = uvRange;

	}
	stbi_write_png("atlas_Color.png", result.atlasWidth, result.atlasHeight, 4, result.ColorData.data(), result.atlasWidth * 4);
	stbi_write_png("atlas_Normal.png", result.atlasWidth, result.atlasHeight, 4, result.NormalData.data(), result.atlasWidth * 4);

	return result;
}

glm::ivec2 World::getChunkCoordinates(glm::vec3 pos) {
	return glm::ivec2(pos.x / CHUNK_SIZE, pos.z / CHUNK_SIZE);
}

void World::setBlock(int x, int y, int z, int blockType) {
	glm::vec3 chunkPos = { x / CHUNK_SIZE, y / CHUNK_SIZE, z / CHUNK_SIZE };
	auto it = chunks.find(chunkPos);
	if (it == chunks.end()) return;

	it->second->dirty = true;
}

int World::getTerrainHeight(int x, int z) {
	float n = heightMap.GetNoise((float)x, (float)z);
	n = (n + 1.0f) * 0.5f;
	return (int)(n * terrainHeight);
}

void World::chunkBuilderLoop() {
	while (chunkBuilderActive) {
		auto requestedChunk = reqChunks.try_pop();
		if (!requestedChunk.has_value()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		glm::ivec3 reqChunkPos = requestedChunk.value();
		std::unique_ptr<Chunk> chunk = generateChunk(reqChunkPos);
		{
			std::lock_guard<std::mutex> lock(stagingMutex);
			stagingChunks[reqChunkPos] = std::move(chunk);
		}
		generatedQueue.push(reqChunkPos);
	}
}

void World::chunkMesherLoop() {
	while (chunkBuilderActive) {
		auto maybeChunkPos = generatedQueue.try_pop();
		if (!maybeChunkPos.has_value()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}
		glm::ivec3 pos = maybeChunkPos.value();
		if (!neighborsReady_local(this, pos)) {
			generatedQueue.push(pos);
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}
		
		Chunk* chunkPtr = nullptr;
		{
			std::lock_guard<std::mutex> lock(stagingMutex);
			auto it = stagingChunks.find(pos);
			if (it == stagingChunks.end()) {
				continue;
			}
			chunkPtr = it->second.get();
		}
		MeshData meshData;
		Mesher(*chunkPtr, meshData.vertices, meshData.indices);

		MeshJob job;
		job.pos = pos;
		job.mesh = std::move(meshData);
		meshedChunks.push(std::move(job));
	}
}

void World::captureGenratedChunks() {
	auto opt = meshedChunks.try_pop();
	while (opt.has_value()) {
		MeshJob job = std::move(opt.value());

		std::unique_ptr<Chunk> chunkPtr;
		{
			std::lock_guard<std::mutex> lock(stagingMutex);
			auto it = stagingChunks.find(job.pos);
			if (it == stagingChunks.end()) {
				opt = meshedChunks.try_pop();
				continue;
			}
			chunkPtr = std::move(it->second);
			stagingChunks.erase(it);
		}
		{
			std::lock_guard<std::mutex> lock(chunkMutex);
			chunks[job.pos] = std::move(chunkPtr);
			chunks[job.pos]->meshData = std::move(job.mesh);
			chunks[job.pos]->dirty = true;
		}
		opt = meshedChunks.try_pop();
	}
}

void World::reqProximityChunks(const glm::vec3& pos) {
	for (int i = -renderDistance; i < renderDistance; i++)
	for (int j = -renderDistance; j < renderDistance; j++) {
		if ((i * i) + (j * j) <= (renderDistance * renderDistance)) {
			glm::ivec3 proxChunk = { pos.x / CHUNK_SIZE + i, 0, pos.z / CHUNK_SIZE + j };
			if (chunks.find(proxChunk) == chunks.end()) reqChunks.push(proxChunk);
		}
	}
}

void World::updateTerrainConstants() {
	heightMap.SetFrequency(terrainScale);
}

void World::clearLoadedChunks() {
	for (auto& [pos, chunkPtr] : chunks) destroyChunkBuffers(*chunkPtr);
	chunks.clear();
}

void World::requestChunk(const glm::ivec3& pos) {
	{
		std::lock_guard<std::mutex> lock1(chunkMutex);
		if (chunks.find(pos) != chunks.end()) return;
	}
	{
		std::lock_guard<std::mutex> lock2(stagingMutex);
		if (stagingChunks.find(pos) != stagingChunks.end()) return;
	}
	reqChunks.push(pos);
}

Chunk* World::findChunk(const glm::ivec3& pos) {
	{
		std::lock_guard<std::mutex> lock1(chunkMutex);
		auto it = chunks.find(pos);
		if (it != chunks.end()) return it->second.get();
	}
	{
		std::lock_guard<std::mutex> lock2(stagingMutex);
		auto it = stagingChunks.find(pos);
		if (it != stagingChunks.end()) return it->second.get();
	}
	return nullptr;
}

bool World::chunkShouldExist(const glm::ivec3& pos) {
	return true;
	int dx = pos.x - playerChunk.x;
	int dz = pos.z - playerChunk.z;
	return ((dx * dx) + (dz * dz)) <= (renderDistance * renderDistance);
}

bool World::neighborsReady_local(World* world, const glm::ivec3& pos) {
	static const glm::ivec3 dirs[4] = { {1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1} };
	for (auto& d : dirs) {
		glm::ivec3 npos = pos + d;
		if (world->chunkShouldExist(npos)) {
			if (!world->findChunk(npos)) return false;
		}
	}
	return true;
}

int World::getChunkCount() {
	return static_cast<uint32_t>(chunks.size());
}