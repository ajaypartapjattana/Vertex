#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

#include "renderer/utility/Vertex.h"
#include "renderer/utility/VulkanUtils.h"

#include "FastNoiseLite.h"

constexpr int CHUNK_SIZE = 16;

struct World_UBO {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;

	alignas(16) glm::vec4 lightDir;
	alignas(16) glm::vec4 lightColor;

	alignas(16) int selected;
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

enum BlockID {
	AIR = 0,
	GRASS = 1,
	STONE = 2,
	DIRT = 3
};

struct BlockData {
	std::string name;
	std::string filePath;
	int index;
};

struct AtlasResult {
	int atlasWidth;
	int atlasHeight;
	int tileSize;
	std::unordered_map<uint8_t, glm::vec4> uvRanges;
	std::vector<unsigned char> pixels;
};

struct Chunk {
	glm::ivec3 chunkPos{};

	uint8_t voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = { 0 };

	Mesh chunkMesh;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
	VkDeviceMemory indexMemory = VK_NULL_HANDLE;

	bool dirty = true;
	bool gpuAllocated = false;
};

struct IVec3Hash {
	std::size_t operator()(const glm::ivec3& v) const noexcept {
		size_t h1 = std::hash<int>()(v.x);
		size_t h2 = std::hash<int>()(v.y);
		size_t h3 = std::hash<int>()(v.z);
		return ((h1 ^ (h2 << 1)) >> 1 ) ^ (h3 << 1);
	}
};

struct IVec3Equal {
	bool operator()(const glm::ivec3& a, const glm::ivec3& b) const noexcept {
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};

class World {
public:
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
	World(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT, const std::string& texture_path);
	~World();

	void generateChunk(const glm::ivec3& pos);
	void updateChunkMesh(const glm::ivec3& pos);
	void uploadChunkToGPU(Chunk& chunk);
	void createTextureImage(const std::string& texture_path);
	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame);
	void updateUBO(VkDevice device, const World_UBO& uboData, uint32_t currentImage);

	void setBlock(int x, int y, int z, int blockType);
	
	void cleanup();

	float heightMultiplier = 10.0f;

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue queue;
	VkCommandPool commandPool;

	FastNoiseLite heightMap;
	AtlasResult atlas;

	VkImage textureAtlas{ VK_NULL_HANDLE };
	VkDeviceMemory textureAtlasMemory{ VK_NULL_HANDLE };
	VkImageView textureAtlasView{ VK_NULL_HANDLE };
	VkSampler textureSampler{ VK_NULL_HANDLE };
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	
	std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash, IVec3Equal> chunks;

	void createDescriptorPool(VkDevice device, uint16_t MAX_FRAMES_IN_FLIGHT);
	void createWorldDescriptorSet(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);
	void createWorldUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, uint16_t MAX_FRAMES_IN_FLIGHT);

	AtlasResult buildTextureAtlas(std::vector<BlockData>& inputBlocks, int tileSize);

	int getTerrainHeight(int x, int z);

	void GreedyMesher(const Chunk& chunk, std::vector<Vertex>& verts, std::vector<uint32_t>& indices);
	void Mesher(const Chunk& chunk, std::vector<Vertex>& verts, std::vector<uint32_t>& indices);

	void createChunkBuffers(Chunk& chunk);
	void destroyChunkBuffers(Chunk& chunk);
};