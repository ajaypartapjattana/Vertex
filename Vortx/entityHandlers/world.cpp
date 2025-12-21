#include "world.h"

#include <stdexcept>
#include <iostream>
#include <random>

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma warning(push)
#pragma warning(disable: 4996)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "renderer/utility/stb_image_write.h"
#pragma warning(pop)

World::World(const ContextHandle& handle) : 
	device(handle.device),
	physicalDevice(handle.physicalDevice),
	descriptorSetLayout(handle.descriptorSetLayout),
	queue(handle.graphicsQueue),
	commandPool(handle.commandPool),
	chunkBuilderActive(true),
	ChunkGenerator(&World::chunkBuilderLoop, this),
	ChunkMesher(&World::chunkMesherLoop, this)
{
	heightMap.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	heightMap.SetFrequency(terrainScale);

	createDescriptorPool(handle.MAX_FRAMES_IN_FLIGHT);

	BlockData grass{};
	grass.name = "grass";
	grass.ColorMap = "block_textures/grass_256.png";
	grass.NormalMap = "block_textures/grass_256_Normal.png";
	grass.index = 2;

	BlockData stone{};
	stone.name = "stone";
	stone.ColorMap = "block_textures/stone_256.png";
	stone.NormalMap = "block_textures/stone_256_Normal.png";
	stone.index = 1;

	BlockData dirt{};
	dirt.name = "dirt";
	dirt.ColorMap = "block_textures/dirt_256.png";
	dirt.NormalMap = "block_textures/dirt_256_Normal.png";
	dirt.index = 3;

	BlockData netherack{};
	netherack.name = "netherack";
	netherack.ColorMap = "block_textures/netherack_256.png";
	netherack.NormalMap = "block_textures/netherack_256_Normal.png";
	netherack.index = 4;

	std::vector<BlockData> inBlocks = { grass , stone, dirt, netherack };
	atlas = buildTextureAtlas(inBlocks, 256);

	colorTexture.format = VK_FORMAT_R8G8B8A8_SRGB;
	colorTexture.samples = VK_SAMPLE_COUNT_1_BIT;
	colorTexture.aspectFlagBits = VK_IMAGE_ASPECT_COLOR_BIT;
	createTextureImage(atlas.ColorData.data(), colorTexture);

	NormalTexture.format = VK_FORMAT_R8G8B8A8_SRGB;
	NormalTexture.samples = VK_SAMPLE_COUNT_1_BIT;
	NormalTexture.aspectFlagBits = VK_IMAGE_ASPECT_COLOR_BIT;
	createTextureImage(atlas.NormalData.data(), NormalTexture);

	VulkanUtils::createTextureSampler(device, physicalDevice, textureSampler, VK_FILTER_NEAREST);

	createWorldUniformBuffer(handle.MAX_FRAMES_IN_FLIGHT);
	createWorldDescriptorSet(descriptorSetLayout, handle.MAX_FRAMES_IN_FLIGHT);
}

World::~World() {
	chunkBuilderActive = false;
	if (ChunkGenerator.joinable()) ChunkGenerator.join();
	if (ChunkMesher.joinable()) ChunkMesher.join();
	cleanup();
}

std::unique_ptr<Chunk> World::generateChunk(const glm::ivec3& pos) {
	if (chunks.find(pos) != chunks.end()) return nullptr;

	//std::cout << "genrating chunk at : [" << pos.x << "," << pos.z << "]" << std::endl;

	auto chunk = std::make_unique<Chunk>();
	chunk->chunkPos = pos;
	chunk->chunkMesh.vertices.clear();
	chunk->chunkMesh.indices.clear();

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

void World::GreedyMesher(const Chunk& chunk, std::vector<Vertex>& verts, std::vector<uint32_t>& indices)
{
	verts.clear();
	indices.clear();

	const glm::ivec3 cpos = chunk.chunkPos;
	const glm::vec3 base = glm::vec3(cpos * CHUNK_SIZE);

	// World-space solid checker
	auto isSolid = [&](int wx, int wy, int wz)
		{
			if (wy < 0 || wy >= CHUNK_SIZE) return false;

			int cx = (int)floor(wx / float(CHUNK_SIZE));
			int cz = (int)floor(wz / float(CHUNK_SIZE));

			glm::ivec3 p(cx, 0, cz);
			auto it = chunks.find(p);
			if (it == chunks.end())
				return false;

			Chunk* ch = it->second.get();

			int lx = wx - cx * CHUNK_SIZE;
			int lz = wz - cz * CHUNK_SIZE;

			if (lx < 0 || lx >= CHUNK_SIZE ||
				lz < 0 || lz >= CHUNK_SIZE)
				return false;

			return ch->voxels[lx][wy][lz] != 0;
		};

	// 6 directions
	const glm::ivec3 normals[6] = {
		{ 1,0,0 }, { -1,0,0 },
		{ 0,1,0 }, { 0,-1,0 },
		{ 0,0,1 }, { 0,0,-1 }
	};

	const glm::ivec3 U[6] = {
		{0,1,0}, {0,1,0},
		{1,0,0}, {1,0,0},
		{1,0,0}, {1,0,0}
	};

	const glm::ivec3 V[6] = {
		{0,0,1}, {0,0,1},
		{0,0,1}, {0,0,1},
		{0,1,0}, {0,1,0}
	};

	// Process each face
	for (int f = 0; f < 6; f++)
	{
		glm::ivec3 n = normals[f];
		glm::ivec3 u = U[f];
		glm::ivec3 v = V[f];

		int du[3] = { u.x, u.y, u.z };
		int dv[3] = { v.x, v.y, v.z };

		int nx = n.x, ny = n.y, nz = n.z;

		uint8_t mask[CHUNK_SIZE * CHUNK_SIZE];

		// Sweep along normal direction
		for (int d = 0; d < CHUNK_SIZE; d++)
		{
			// Build mask
			int m = 0;
			for (int j = 0; j < CHUNK_SIZE; j++)
			{
				for (int i = 0; i < CHUNK_SIZE; i++)
				{
					int x = nx ? (nx > 0 ? d : CHUNK_SIZE - 1 - d) : i;
					int y = ny ? (ny > 0 ? d : CHUNK_SIZE - 1 - d) : j;
					int z = nz ? (nz > 0 ? d : CHUNK_SIZE - 1 - d) : (nx ? j : i);

					uint8_t block = chunk.voxels[x][y][z];

					if (!block)
					{
						mask[m++] = 0;
						continue;
					}

					int wx = cpos.x * CHUNK_SIZE + x;
					int wy = y;
					int wz = cpos.z * CHUNK_SIZE + z;

					int ax = wx + nx;
					int ay = wy + ny;
					int az = wz + nz;

					mask[m++] = isSolid(ax, ay, az) ? 0 : block;
				}
			}

			// Greedy merge
			m = 0;
			for (int j = 0; j < CHUNK_SIZE; j++)
			{
				for (int i = 0; i < CHUNK_SIZE;)
				{
					uint8_t block = mask[m];
					if (!block)
					{
						++i; ++m;
						continue;
					}

					int w = 1;
					while (i + w < CHUNK_SIZE && mask[m + w] == block)
						++w;

					int h = 1;
					bool stop = false;
					while (j + h < CHUNK_SIZE)
					{
						for (int k = 0; k < w; k++)
						{
							if (mask[m + k + h * CHUNK_SIZE] != block)
							{
								stop = true;
								break;
							}
						}
						if (stop) break;
						++h;
					}

					// Emit quad
					glm::vec4 uv = atlas.uvRanges.at(block);

					glm::vec3 p0 =
						base +
						glm::vec3(nx ? (nx > 0 ? d : d + 1) : i,
							ny ? (ny > 0 ? d : d + 1) : j,
							nz ? (nz > 0 ? d : d + 1) : 0);

					glm::vec3 pos = glm::vec3(cpos * CHUNK_SIZE);

					glm::vec3 o =
						glm::vec3(
							nx ? (nx > 0 ? d : CHUNK_SIZE - d) : i,
							ny ? (ny > 0 ? d : CHUNK_SIZE - d) : j,
							nz ? (nz > 0 ? d : CHUNK_SIZE - d) : 0
						);

					glm::vec3 basePos = base + o;

					glm::vec3 p[4] = {
						basePos,
						basePos + glm::vec3(u) * (float)w,
						basePos + glm::vec3(v) * (float)h + glm::vec3(u) * (float)w,
						basePos + glm::vec3(v) * (float)h
					};

					uint32_t bi = static_cast<uint32_t>(verts.size());

					glm::vec2 uvFace[4] = {
						{uv.x, uv.y},
						{uv.z, uv.y},
						{uv.z, uv.w},
						{uv.x, uv.w}
					};

					for (int q = 0; q < 4; q++)
					{
						Vertex vert;
						vert.pos = p[q];
						vert.normal = glm::vec3(n);
						vert.color = { 1,1,1 };
						vert.texCoord = uvFace[q];
						verts.push_back(vert);
					}

					indices.push_back(bi + 0);
					indices.push_back(bi + 1);
					indices.push_back(bi + 2);
					indices.push_back(bi + 2);
					indices.push_back(bi + 3);
					indices.push_back(bi + 0);

					// Clear merged area
					for (int a = 0; a < h; a++)
						for (int b = 0; b < w; b++)
							mask[m + b + a * CHUNK_SIZE] = 0;

					i += w;
					m += w;
				}
			}
		}
	}
}




void World::uploadChunkToGPU(Chunk& chunk) {
	if (!chunk.dirty && chunk.chunkMesh.vertices.empty()) return;

	VkDeviceSize vertexSize = sizeof(Vertex) * chunk.chunkMesh.vertices.size();
	VkDeviceSize indexSize = sizeof(uint32_t) * chunk.chunkMesh.indices.size();

	VkBuffer stagingVertexBuffer, stagingIndexBuffer;
	VkDeviceMemory stagingVertexmemory, stagingIndexMemory;

	VulkanUtils::createBuffer(physicalDevice, device, vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingVertexBuffer, stagingVertexmemory);
	VulkanUtils::createBuffer(physicalDevice, device, indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndexBuffer, stagingIndexMemory);

	void* data;
	vkMapMemory(device, stagingVertexmemory, 0, vertexSize, 0, &data);
	memcpy(data, chunk.chunkMesh.vertices.data(), (size_t)vertexSize);
	vkUnmapMemory(device, stagingVertexmemory);

	vkMapMemory(device, stagingIndexMemory, 0, indexSize, 0, &data);
	memcpy(data, chunk.chunkMesh.indices.data(), (size_t)indexSize);
	vkUnmapMemory(device, stagingIndexMemory);

	VulkanUtils::createBuffer(physicalDevice, device, vertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, chunk.vertexBuffer, chunk.vertexMemory);
	VulkanUtils::createBuffer(physicalDevice, device, indexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, chunk.indexBuffer, chunk.indexMemory);
	VulkanUtils::copyBuffer(commandPool, device, stagingVertexBuffer, chunk.vertexBuffer, vertexSize, queue);
	VulkanUtils::copyBuffer(commandPool, device, stagingIndexBuffer, chunk.indexBuffer, indexSize, queue);
		
	vkDestroyBuffer(device, stagingVertexBuffer, nullptr);
	vkDestroyBuffer(device, stagingIndexBuffer, nullptr);
	vkFreeMemory(device, stagingVertexmemory, nullptr);
	vkFreeMemory(device, stagingIndexMemory, nullptr);

	chunk.dirty = false;
	chunk.gpuAllocated = true;
}

void World::updateChunkMesh(const glm::ivec3& pos) {
	auto it = chunks.find(pos);
	if (it == chunks.end()) return;
	Chunk& chunk = *it->second;
	destroyChunkBuffers(chunk);
	uploadChunkToGPU(chunk);
}

TextureAtlas World::buildTextureAtlas(std::vector<BlockData>& inputBlocks, int tileSize) {
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

		//std::cout << block.name << "[" << block.index << "]" << "uvMin: " << u0 << "," << v0 << "; uvMax: " << u1 << v1 << std::endl;
	}
	stbi_write_png("atlas_Color.png", result.atlasWidth, result.atlasHeight, 4, result.ColorData.data(), result.atlasWidth * 4);
	stbi_write_png("atlas_Normal.png", result.atlasWidth, result.atlasHeight, 4, result.NormalData.data(), result.atlasWidth * 4);

	return result;
}

void World::createTextureImage(unsigned char* imageData, ImageResources& image) {
	VkDeviceSize imageSize = atlas.atlasWidth * atlas.atlasHeight * 4;

	if (!imageData) {
		throw std::runtime_error("failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VulkanUtils::createBuffer(physicalDevice, device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	VulkanUtils::createImageResources(device, physicalDevice, atlas.atlasWidth, atlas.atlasHeight, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);
	VulkanUtils::transitionImageLayout(commandPool, device, queue, image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VulkanUtils::copyBufferToImage(device, queue, commandPool, stagingBuffer, image.image, static_cast<uint32_t>(atlas.atlasWidth), static_cast<uint32_t>(atlas.atlasHeight));
	VulkanUtils::transitionImageLayout(commandPool, device, queue, image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	//imageView = VulkanUtils::createImageView(device, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void World::updateUBO(VkDevice device, const World_UBO& uboData, uint32_t currentImage) {
	memcpy(uniformBuffersMapped[currentImage], &uboData, sizeof(uboData));
}

void World::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame) {
	if (chunks.empty()) return;
	for (auto& [pos, chunkPtr] : chunks) {
		Chunk& chunk = *chunkPtr;
		if (chunk.dirty) uploadChunkToGPU(chunk);
		if (!chunk.gpuAllocated) continue;

		VkBuffer vertexBuffers[] = { chunk.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, chunk.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		if (!descriptorSets.empty()) {
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
		}

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(chunk.chunkMesh.indices.size()), 1, 0, 0, 0);
	}
}

//int World::getSurfaceZ(glm::vec3 pos) {
//	glm::ivec2 ChunkCoordinates = getChunkCoordinates(pos);
//	auto& chunk = chunks[glm::ivec3(ChunkCoordinates.x, 0, ChunkCoordinates.y)];
//
//	for (int y = pos.y; y > 0; y--) {
//		if(chunk->voxels[])
//	}
//}

glm::ivec2 World::getChunkCoordinates(glm::vec3 pos) {
	return glm::ivec2(pos.x / CHUNK_SIZE, pos.z / CHUNK_SIZE);
}

void World::setBlock(int x, int y, int z, int blockType) {
	glm::vec3 chunkPos = { x / CHUNK_SIZE, y / CHUNK_SIZE, z / CHUNK_SIZE };
	auto it = chunks.find(chunkPos);
	if (it == chunks.end()) return;

	it->second->dirty = true;
}

void World::cleanup() {
	for (auto& [pos, chunkPtr] : chunks) destroyChunkBuffers(*chunkPtr);
	chunks.clear();

	for (size_t i = 0; i < descriptorSets.size(); ++i) {
		if (uniformBuffersMapped[i]) {
			vkUnmapMemory(device, uniformBuffersMemory[i]);
			uniformBuffersMapped[i] = nullptr;
		}
		if (uniformBuffers[i] != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
			uniformBuffers[i] = VK_NULL_HANDLE;
		}
	}
	uniformBuffers.clear();
	uniformBuffersMemory.clear();
	uniformBuffersMapped.clear();
	descriptorSets.clear();

	if (descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		descriptorPool = VK_NULL_HANDLE;
	}

	if (textureSampler != VK_NULL_HANDLE) {
		vkDestroySampler(device, textureSampler, nullptr);
		textureSampler = VK_NULL_HANDLE;
	}
	VulkanUtils::destroyImageResources(device, colorTexture);
	VulkanUtils::destroyImageResources(device, NormalTexture);
}

void World::createChunkBuffers(Chunk& chunk) {
	uploadChunkToGPU(chunk);
}

void World::destroyChunkBuffers(Chunk& chunk) {
	vkDeviceWaitIdle(device);
	if (chunk.vertexBuffer) vkDestroyBuffer(device, chunk.vertexBuffer, nullptr);
	if (chunk.indexBuffer) vkDestroyBuffer(device, chunk.indexBuffer, nullptr);
	if (chunk.vertexMemory) vkFreeMemory(device, chunk.vertexMemory, nullptr);
	if (chunk.indexMemory) vkFreeMemory(device, chunk.indexMemory, nullptr);
	chunk.vertexBuffer = VK_NULL_HANDLE;
	chunk.indexBuffer = VK_NULL_HANDLE;
	chunk.vertexMemory = VK_NULL_HANDLE;
	chunk.indexMemory = VK_NULL_HANDLE;
	chunk.gpuAllocated = false;
}

void World::createDescriptorPool(uint16_t MAX_FRAMES_IN_FLIGHT) {
	std::array<VkDescriptorPoolSize, 3> poolSizes;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void World::createWorldDescriptorSet(VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT) {
	std::vector<VkDescriptorSetLayout> layouts(FRAMES_IN_FLIGHT, descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate world descriptor set!");
	}

	for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(World_UBO);

		VkDescriptorImageInfo ColorMapInfo{};
		ColorMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ColorMapInfo.imageView = colorTexture.view;
		ColorMapInfo.sampler = textureSampler;

		VkDescriptorImageInfo NormalMapInfo{};
		NormalMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		NormalMapInfo.imageView = NormalTexture.view;
		NormalMapInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &ColorMapInfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &NormalMapInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void World::createWorldUniformBuffer(uint16_t MAX_FRAMES_IN_FLIGHT) {
	VkDeviceSize bufferSize = sizeof(World_UBO);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VulkanUtils::createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
		vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
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
		Mesh mesh;
		Mesher(*chunkPtr, mesh.vertices, mesh.indices);

		MeshJob job;
		job.pos = pos;
		job.mesh = std::move(mesh);
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
			chunks[job.pos]->chunkMesh = std::move(job.mesh);
			chunks[job.pos]->dirty = true;
		}
		//possible here - chunk upload code.
		opt = meshedChunks.try_pop();
	}
}

void World::reqProximityChunks(const glm::vec3& pos) {
	for (int i = -renderDistance; i < renderDistance; i++)
	for (int j = -renderDistance; j < renderDistance; j++) {
		if ((i * i) + (j * j) <= (renderDistance * renderDistance)) {
			glm::ivec3 proxChunk = { pos.x / CHUNK_SIZE + i, 0, pos.z / CHUNK_SIZE + j };
			if (chunks.find(proxChunk) == chunks.end()) reqChunks.push(proxChunk);
			//std::cout << "requested chunk : [" << proxChunk.x << "," << proxChunk.z << "]" << std::endl;
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