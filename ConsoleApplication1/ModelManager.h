#pragma once

#include "model.h"
#include <memory>
#include <vector>
#include <string>

class ModelManager {
public:
	void loadModel(const std::string& path, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue);
	void drawAll(VkCommandBuffer commandBuffer);
	void cleanUp(VkDevice device);

private:
	std::vector<std::unique_ptr<Model>> models;
};