#pragma once

#include "model.h"
#include <memory>
#include <vector>
#include <string>
#include <fstream>

class ModelManager {
public:
	void loadModel(const std::string& obj_path, const std::string& texture_path, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, uint16_t FRAMES_IN_FLIGHT);
	void createModelDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);
	void drawAll(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame);
	void cleanUp(VkDevice device);
	Model* getModel(size_t index);
	std::vector<std::unique_ptr<Model>>& getModelList();
	void saveModelMeta();
	void loadModelMeta();

	int selectedModelIndex = -1;

private:
	std::vector<std::unique_ptr<Model>> models;
};