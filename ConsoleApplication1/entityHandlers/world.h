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

#include "renderer/VulkanContext.h"

#include "FastNoiseLite.h"

struct World_UBO {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;

	alignas(16) glm::vec4 lightDir;
	alignas(16) glm::vec4 lightColor;

	alignas(16) glm::vec4 cameraPos;

	alignas(16) glm::vec4 sphereInfo;

	alignas(16) int selected;
	int _pad[3];
};

struct WorldRenderState {
	float worldCurvature;
	float radius;
};

struct BlockData {
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
	std::vector<unsigned char> ColorData;
	std::vector<unsigned char> NormalData;
};

struct genratedChunk {
	glm::ivec3 pos;
	std::unique_ptr<Chunk> chunk;
};

class World {
public:
	World(const ContextHandle& handle);
	~World();

	int getChunkCount();
	void reqProximityChunks(const glm::vec3& pos);
	void captureGenratedChunks();
	void updateTerrainConstants();
	void clearLoadedChunks();

	bool chunkShouldExist(const glm::ivec3& pos);
	Chunk* findChunk(const glm::ivec3& pos);

	void updateChunkMesh(const glm::ivec3& pos);
	void uploadChunkToGPU(Chunk& chunk);
	void createTextureImage(unsigned char* data, VkImage& image, VkDeviceMemory& imageMemory, VkImageView& imageView);
	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame);
	void updateUBO(VkDevice device, const World_UBO& uboData, uint32_t currentImage);

	int getSurfaceZ(glm::vec3 pos);
	void setBlock(int x, int y, int z, int blockType);
	
	void cleanup();

	uint32_t worldSpherePipeline;

	WorldRenderState renderState;

	float terrainHeight = 50.0f;
	float terrainScale = 0.01f;

	glm::ivec3 playerChunk = { 0,0,0 };
	int renderDistance = 8;

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue queue;
	VkCommandPool commandPool;
	VkDescriptorSetLayout descriptorSetLayout;

	FastNoiseLite heightMap;
	TextureAtlas atlas;

	VkImage C_TextureAtlas{ VK_NULL_HANDLE };
	VkDeviceMemory C_TextureMemory{ VK_NULL_HANDLE };
	VkImageView C_TextureAtlasView{ VK_NULL_HANDLE };

	VkImage N_TextureAtlas{ VK_NULL_HANDLE };
	VkDeviceMemory N_TextureMemory{ VK_NULL_HANDLE };
	VkImageView N_TextureAtlasView{ VK_NULL_HANDLE };

	VkSampler textureSampler{ VK_NULL_HANDLE };
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	
	void createDescriptorPool(uint16_t MAX_FRAMES_IN_FLIGHT);
	void createWorldDescriptorSet(VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);
	void createWorldUniformBuffer(uint16_t MAX_FRAMES_IN_FLIGHT);

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