#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

#include "Vertex.h"

class Model {
public:
	Model () = default;
	~Model ();

	void loadFromFile(const std::string& path);
	void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue);
	void cleanup(VkDevice device);
	void draw(VkCommandBuffer commandBuffer);

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory vertexBufferMemory{ VK_NULL_HANDLE };
	VkBuffer indexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory indexBufferMemory{ VK_NULL_HANDLE };
};