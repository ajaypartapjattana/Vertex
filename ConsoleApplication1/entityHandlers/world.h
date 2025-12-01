#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

#include "renderer/utility/Vertex.h"
#include "renderer/utility/Chunk.h"
#include "renderer/utility/VulkanUtils.h"
#include "commProtocols/threadCommProtocol.h"

#include "FastNoiseLite.h"

struct World_UBO {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;

	alignas(16) glm::vec4 lightDir;
	alignas(16) glm::vec4 lightColor;

	alignas(16) int selected;
};

struct BlockData {
	std::string name;
	std::string filePath;
	int index;
};

struct TextureAtlas {
	int atlasWidth;
	int atlasHeight;
	int tileSize;
	std::unordered_map<uint8_t, glm::vec4> uvRanges;
	std::vector<unsigned char> pixels;
};

struct genratedChunk {
	glm::ivec3 pos;
	std::unique_ptr<Chunk> chunk;
};

class World {
public:
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
	World(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);
	~World();

	int getChunkCount();
	void reqProximityChunks(const glm::vec3& pos);
	void captureGenratedChunks();

	bool chunkShouldExist(const glm::ivec3& pos);
	Chunk* findChunk(const glm::ivec3& pos);

	void updateChunkMesh(const glm::ivec3& pos);
	void uploadChunkToGPU(Chunk& chunk);
	void createTextureImage(TextureAtlas atlas);
	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame);
	void updateUBO(VkDevice device, const World_UBO& uboData, uint32_t currentImage);

	int getSurfaceZ(glm::vec3 pos);
	void setBlock(int x, int y, int z, int blockType);
	
	void cleanup();

	float heightMultiplier = 50.0f;

	glm::ivec3 playerChunk = { 0,0,0 };
	int renderDistance = 32;

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue queue;
	VkCommandPool commandPool;

	FastNoiseLite heightMap;
	TextureAtlas atlas;

	VkImage textureAtlas{ VK_NULL_HANDLE };
	VkDeviceMemory textureAtlasMemory{ VK_NULL_HANDLE };
	VkImageView textureAtlasView{ VK_NULL_HANDLE };
	VkSampler textureSampler{ VK_NULL_HANDLE };
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	
	void createDescriptorPool(VkDevice device, uint16_t MAX_FRAMES_IN_FLIGHT);
	void createWorldDescriptorSet(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);
	void createWorldUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, uint16_t MAX_FRAMES_IN_FLIGHT);

	TextureAtlas buildTextureAtlas(std::vector<BlockData>& inputBlocks, int tileSize);

	int getTerrainHeight(int x, int z);
	glm::ivec2 getChunkCoordinates(glm::vec3 pos);

	void GreedyMesher(const Chunk& chunk, std::vector<Vertex>& verts, std::vector<uint32_t>& indices);
	void Mesher(const Chunk& chunk, std::vector<Vertex>& verts, std::vector<uint32_t>& indices);

	void createChunkBuffers(Chunk& chunk);
	void destroyChunkBuffers(Chunk& chunk);

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