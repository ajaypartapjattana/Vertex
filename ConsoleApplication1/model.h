#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

#include "Vertex.h"

struct ModelAttribs {
	size_t trisCount;
	size_t vertCount;
	size_t uniqueVertCount;
};

struct Transform {
	glm::vec3 position { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale	   { 1.0f, 1.0f, 1.0f };
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;

	alignas(16) glm::vec4 lightDir;
	alignas(16) glm::vec4 lightColor;
};

class Model {
public:
	Model() = default;
	~Model();

	ModelAttribs loadedModelAttributes;

	Transform modelTransforms;

	void loadFromFile(const std::string& path);
	void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue);
	void createDescriptorPool(VkDevice device, uint16_t MAX_FRAMES_IN_FLIGHT);
	void createUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, uint16_t MAX_FRAMES_IN_FLIGHT);
	void updateUBO(VkDevice device, const UniformBufferObject& uboData, uint32_t currentImage);
	void createTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, const std::string& texture_path);
	void createDescriptorSet(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);
	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame);
	void cleanup(VkDevice device);

private:
	ModelAttribs loadedModelAttributes;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory vertexBufferMemory{ VK_NULL_HANDLE };
	VkBuffer indexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory indexBufferMemory{ VK_NULL_HANDLE };

	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	VkImage textureImage{ VK_NULL_HANDLE };
	VkDeviceMemory textureImageMemory{ VK_NULL_HANDLE };
	VkImageView textureImageView{ VK_NULL_HANDLE };
	VkSampler textureSampler{ VK_NULL_HANDLE };

	std::vector<VkDescriptorSet> descriptorSets;
};