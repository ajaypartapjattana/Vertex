#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

#include "renderer/utility/Vertex.h"
#include "renderer/utility/VulkanUtils.h"

constexpr int CHUNK_SIZE = 16;

struct World_UBO {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;

	alignas(16) glm::vec4 lightDir;
	alignas(16) glm::vec4 lightColor;
};

struct Chunk {
	glm::ivec3 chunkPos{};

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
	VkDeviceMemory indexMemory = VK_NULL_HANDLE;

	bool dirty = true;
	bool gpuAllocated = false;
};

struct Vec3Hash {
	std::size_t operator()(const glm::vec3& v) const noexcept {
		return ((std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1)) >> 1 ) ^ (std::hash<int>()(v.z) << 1);
	}
};

struct Vec3Equal {
	bool operator()(const glm::vec3& a, const glm::vec3& b) const noexcept {
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};

class World {
public:
	World(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);
	~World();

	void generateChunk(const glm::ivec3& pos);
	void updateChunkMesh(const glm::ivec3& pos);
	void uploadChunkToGPU(Chunk& chunk);
	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame);

	void setBlock(int x, int y, int z, int blockType);
	
	void cleanup();

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue queue;
	VkCommandPool commandPool;

	VkImage textureImage{ VK_NULL_HANDLE };
	VkDeviceMemory textureImageMemory{ VK_NULL_HANDLE };
	VkImageView textureImageView{ VK_NULL_HANDLE };
	VkSampler textureSampler{ VK_NULL_HANDLE };
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	
	std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, Vec3Hash, Vec3Equal> chunks;

	void createDescriptorPool(VkDevice device, uint16_t MAX_FRAMES_IN_FLIGHT);
	void createWorldDescriptorSet(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);
	void createWorldUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, uint16_t MAX_FRAMES_IN_FLIGHT);

	void createChunkBuffers(Chunk& chunk);
	void destroyChunkBuffers(Chunk& chunk);
};