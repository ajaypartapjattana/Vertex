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

World::World(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT, const std::string& texture_path)
	: device(device), physicalDevice(physicalDevice), queue(queue), commandPool(commandPool) {
	heightMap.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	heightMap.SetFrequency(0.1f);

	createDescriptorPool(device, FRAMES_IN_FLIGHT);

	BlockData grass{};
	grass.name = "grass";
	grass.index = 2;
	grass.filePath = "block_textures/grass.png";

	BlockData stone{};
	stone.name = "stone";
	stone.index = 1;
	stone.filePath = "block_textures/stone.png";

	BlockData dirt{};
	dirt.name = "dirt";
	dirt.index = 3;
	dirt.filePath = "block_textures/dirt.png";

	BlockData netherack{};
	netherack.name = "netherack";
	netherack.index = 4;
	netherack.filePath = "block_textures/netherack.png";

	std::vector<BlockData> inBlocks = { grass , stone, dirt, netherack };
	atlas = buildTextureAtlas(inBlocks, 16);

	createTextureImage("atlas.png");
	createWorldUniformBuffer(device, physicalDevice, FRAMES_IN_FLIGHT);
	createWorldDescriptorSet(device, descriptorSetLayout, FRAMES_IN_FLIGHT);
}

World::~World() {
	cleanup();
}

void World::generateChunk(const glm::ivec3& pos) {
	if (chunks.find(pos) != chunks.end()) return;

	auto chunk = std::make_unique<Chunk>();
	chunk->chunkPos = pos;
	chunk->chunkMesh.vertices.clear();
	chunk->chunkMesh.indices.clear();

	//chunk genration:
	int baseX = pos.x * CHUNK_SIZE;
	int baseZ = pos.z * CHUNK_SIZE;

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++) {
			int worldX = baseX + x;
			int worldZ = baseZ + z;

			int terrainHeight = getTerrainHeight(worldX, worldZ);

			for (int y = 0; y < CHUNK_SIZE; y++) {
				if (y < terrainHeight - 3) chunk->voxels[x][y][z] = 1;
				else if (y < terrainHeight) chunk->voxels[x][y][z] = 3;
				else if (y == terrainHeight) chunk->voxels[x][y][z] = 2;
				else chunk->voxels[x][y][z] = 0;
			}
		}

	//GreedyMesher(*chunk, chunk->chunkMesh.vertices, chunk->chunkMesh.indices);
	Mesher(*chunk, chunk->chunkMesh.vertices, chunk->chunkMesh.indices);

	std::cout << "verts = " << chunk->chunkMesh.vertices.size() << " indices = " << chunk->chunkMesh.indices.size() << std::endl;

	chunk->dirty = true;
	chunks[pos] = std::move(chunk);
}

void World::Mesher(const Chunk& chunk, std::vector<Vertex>& verts, std::vector<uint32_t>& indices)
{
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

	const glm::vec3 faceVertices[6][4] = {
		{ {1,0,0}, {1,1,0}, {1,1,1}, {1,0,1} },
		{ {0,0,1}, {0,1,1}, {0,1,0}, {0,0,0} },
		{ {0,1,1}, {1,1,1}, {1,1,0}, {0,1,0} },
		{ {0,0,0}, {1,0,0}, {1,0,1}, {0,0,1} },
		{ {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1} },
		{ {1,0,0}, {0,0,0}, {0,1,0}, {1,1,0} }
	};

	auto isSolid = [&](int x, int y, int z) {
		if (x < 0 || y < 0 || z < 0) return false;
		if (x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE) return false;
		return chunk.voxels[x][y][z] != 0;
		};

	for (int x = 0; x < CHUNK_SIZE; x++){
		for (int y = 0; y < CHUNK_SIZE; y++){
			for (int z = 0; z < CHUNK_SIZE; z++){
				//if (!isSolid(x, y, z)) continue;
				uint8_t block = chunk.voxels[x][y][z];
				if (!block) continue;

				glm::vec4 uvRect = atlas.uvRanges.at(block);

				glm::vec2 uvFace[4] = {
					{uvRect.x, uvRect.y},
					{uvRect.z, uvRect.y},
					{uvRect.z, uvRect.w},
					{uvRect.x, uvRect.w}
				};

				for (int f = 0; f < 6; f++){
					glm::ivec3 n = faceNormals[f];
					int nx = x + n.x, ny = y + n.y, nz = z + n.z;
					if (isSolid(nx, ny, nz))
						continue;

					uint32_t baseIndex = static_cast<uint32_t>(verts.size());
					for (int v = 0; v < 4; v++){
						Vertex vert;
						vert.pos = glm::vec3(x, y, z) + faceVertices[f][v] + glm::vec3(chunk.chunkPos * CHUNK_SIZE);
						vert.normal = glm::vec3(n);
						vert.color = {1.0f,1.0f,1.0f};
						vert.texCoord = uvFace[v];
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

	auto isSolid = [&](int x, int y, int z) {
		if (x < 0 || y < 0 || z < 0) return false;
		if (x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE) return false;
		return chunk.voxels[x][y][z] != 0;
		};

	// Face directions (axis, direction)
	struct Axis { int u, v, w; }; // u=0/1/2 means x/y/z

	// 3 axis sweeps: Z, Y, X
	const Axis axes[3] = {
		{0, 1, 2},  // sweep Z, build XY quads
		{0, 2, 1},  // sweep Y, build XZ quads
		{1, 2, 0}   // sweep X, build YZ quads
	};

	for (int a = 0; a < 3; a++)
	{
		int U = axes[a].u;
		int V = axes[a].v;
		int W = axes[a].w;

		int size[3] = { CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE };

		// Allocate a mask for the current slice
		std::vector<int> mask(size[U] * size[V]);

		// Sweep the slice along W axis
		for (int w = -1; w < size[W]; w++)
		{
			// Build mask
			int n = 0;
			for (int v = 0; v < size[V]; v++)
			{
				for (int u = 0; u < size[U]; u++, n++)
				{
					int a0[3] = { 0,0,0 };
					int a1[3] = { 0,0,0 };

					a0[W] = w;
					a1[W] = w + 1;
					a0[U] = a1[U] = u;
					a0[V] = a1[V] = v;

					bool s0 = (w >= 0) && isSolid(a0[0], a0[1], a0[2]);
					bool s1 = (w + 1 < size[W]) && isSolid(a1[0], a1[1], a1[2]);

					if (s0 == s1)
						mask[n] = 0;     // no face between identical solidity
					else
						mask[n] = s0 ? 1 : -1;  // 1 = face pointing +W, -1 = face -W
				}
			}

			// Greedy merge the mask into big rectangles
			n = 0;
			for (int v = 0; v < size[V]; v++)
			{
				for (int u = 0; u < size[U]; )
				{
					int c = mask[u + v * size[U]];
					if (c == 0) {
						u++;
						continue;
					}

					// Find width
					int width = 1;
					while (u + width < size[U] && mask[u + width + v * size[U]] == c)
						width++;

					// Find height
					int height = 1;
					bool done = false;
					while (v + height < size[V])
					{
						for (int k = 0; k < width; k++)
						{
							if (mask[u + k + (v + height) * size[U]] != c)
							{
								done = true;
								break;
							}
						}
						if (done) break;
						height++;
					}

					// Generate quad from (u,v) sized (width,height)
					int x[3] = { 0,0,0 };
					int du[3] = { 0,0,0 };
					int dv[3] = { 0,0,0 };

					x[U] = u;      x[V] = v;      x[W] = (c > 0 ? w + 1 : w);
					du[U] = width; du[V] = 0;      du[W] = 0;
					dv[U] = 0;     dv[V] = height; dv[W] = 0;

					glm::vec3 p0(x[0], x[1], x[2]);
					glm::vec3 p1(x[0] + du[0], x[1] + du[1], x[2] + du[2]);
					glm::vec3 p2(x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2]);
					glm::vec3 p3(x[0] + dv[0], x[1] + dv[1], x[2] + dv[2]);

					glm::vec3 normal(0);
					normal[W] = (c > 0 ? 1.0f : -1.0f);

					uint32_t base = verts.size();
					glm::vec3 color = {1.0f, 1.0f, 1.0f};

					verts.push_back({ p0, normal, color, {0,0} });
					verts.push_back({ p1, normal, color, {1,0} });
					verts.push_back({ p2, normal, color, {1,1} });
					verts.push_back({ p3, normal, color, {0,1} });

					indices.push_back(base + 0);
					indices.push_back(base + 1);
					indices.push_back(base + 2);
					indices.push_back(base + 2);
					indices.push_back(base + 3);
					indices.push_back(base + 0);

					// Clear the mask for the consumed area
					for (int hv = 0; hv < height; hv++)
					{
						for (int hu = 0; hu < width; hu++)
						{
							mask[(u + hu) + (v + hv) * size[U]] = 0;
						}
					}

					u += width;
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

	if (World::vkSetDebugUtilsObjectNameEXT) {
		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.objectHandle = (uint64_t)chunk.vertexBuffer;
		nameInfo.pObjectName = "chunkVertexBuffer";
		vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
	}
		

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

AtlasResult World::buildTextureAtlas(std::vector<BlockData>& inputBlocks, int tileSize) {
	AtlasResult result;
	result.tileSize = tileSize;
	int blockCount = inputBlocks.size();

	int atlasTilesPerRow = (int)std::ceil(std::sqrt(blockCount));
	int atlasSizePx = atlasTilesPerRow * tileSize;

	result.atlasWidth = result.atlasHeight = atlasSizePx;

	result.pixels.resize(atlasSizePx * atlasSizePx * 4);
	std::fill(result.pixels.begin(), result.pixels.end(), 0);

	int index = 0;
	for (auto& block : inputBlocks) {
		int xTile = index % atlasTilesPerRow;
		int yTile = index / atlasTilesPerRow;

		//block.index = 
		index++;

		int dstX = xTile * tileSize;
		int dstY = yTile * tileSize;

		int w, h, comp;
		unsigned char* data = stbi_load(block.filePath.c_str(), &w, &h, &comp, 4);

		if (!data) {
			std::cerr << "failed to load block: " << block.filePath << "\n";
			continue;
		}
		if (w != tileSize || h != tileSize) {
			std::cerr << "Warning: " << block.filePath << " not " << tileSize << "x" << tileSize << "\n";
		}

		for (int yy = 0; yy < tileSize; yy++) {
			for (int xx = 0; xx < tileSize; xx++) {
				int srcIndex = (yy * tileSize + xx) * 4;
				int dstIndex = ((dstY + yy) * atlasSizePx + (dstX + xx)) * 4;
				result.pixels[dstIndex + 0] = data[srcIndex + 0];
				result.pixels[dstIndex + 1] = data[srcIndex + 1];
				result.pixels[dstIndex + 2] = data[srcIndex + 2];
				result.pixels[dstIndex + 3] = data[srcIndex + 3];
			}
		}

		stbi_write_png("atlas.png", result.atlasWidth, result.atlasHeight, 4, result.pixels.data(), result.atlasWidth * 4);

		stbi_image_free(data);
		float px = (float)atlasSizePx;
		float inset = 0.5f / px;

		float u0 = (dstX + inset) / px;
		float v0 = (dstY + inset) / px;
		float u1 = (dstX + tileSize - inset) / px;
		float v1 = (dstY + tileSize - inset) / px;

		glm::vec4 uvRange = { u0,v0,u1,v1 };
		result.uvRanges[block.index] = uvRange;

		std::cout << block.name << "[" << block.index << "]" << "uvMin: " << u0 << "," << v0 << "; uvMax: " << u1 << v1 << std::endl;
	}
	return result;
}


void World::createTextureImage(const std::string& texturePath) {
	int texWidth, texHeight, texChannel;
	stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannel, 4);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VulkanUtils::createBuffer(physicalDevice, device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);
	stbi_image_free(pixels);

	VulkanUtils::createImage(device, physicalDevice, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureAtlas, textureAtlasMemory);
	VulkanUtils::transitionImageLayout(commandPool, device, queue, textureAtlas, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VulkanUtils::copyBufferToImage(device, queue, commandPool, stagingBuffer, textureAtlas, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	VulkanUtils::transitionImageLayout(commandPool, device, queue, textureAtlas, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	textureAtlasView = VulkanUtils::createImageView(device, textureAtlas, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	VulkanUtils::createTextureSampler(device, physicalDevice, textureSampler);
}

void World::updateUBO(VkDevice device, const World_UBO& uboData, uint32_t currentImage) {
	memcpy(uniformBuffersMapped[currentImage], &uboData, sizeof(uboData));
}

void World::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame) {
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

void World::setBlock(int x, int y, int z, int blockType) {
	glm::vec3 chunkPos = { x / CHUNK_SIZE, y / CHUNK_SIZE, z / CHUNK_SIZE };
	auto it = chunks.find(chunkPos);
	if (it == chunks.end()) return;

	it->second->dirty = true;
}

void World::cleanup() {
	for (auto& [pos, chunkPtr] : chunks) destroyChunkBuffers(*chunkPtr);
	chunks.clear();
	//std::cout << "buffer_removed" << std::endl;

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
	if (textureAtlasView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, textureAtlasView, nullptr);
		textureAtlasView = VK_NULL_HANDLE;
	}
	if (textureAtlas != VK_NULL_HANDLE) {
		vkDestroyImage(device, textureAtlas, nullptr);
		vkFreeMemory(device, textureAtlasMemory, nullptr);
		textureAtlasMemory = VK_NULL_HANDLE;
	}
}

void World::createChunkBuffers(Chunk& chunk) {
	uploadChunkToGPU(chunk);
}

void World::destroyChunkBuffers(Chunk& chunk) {
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

void World::createDescriptorPool(VkDevice device, uint16_t MAX_FRAMES_IN_FLIGHT) {
	std::array<VkDescriptorPoolSize, 2> poolSizes;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void World::createWorldDescriptorSet(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT) {
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

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureAtlasView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
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
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void World::createWorldUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, uint16_t MAX_FRAMES_IN_FLIGHT) {
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
	return (int)(n * heightMultiplier);
}