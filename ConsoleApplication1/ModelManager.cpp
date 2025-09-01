#include "ModelManager.h"

void ModelManager::loadModel(const std::string& obj_path, const std::string& texture_path, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, uint16_t FRAMES_IN_FLIGHT) {
	auto model = std::make_unique<Model>();
	model->loadFromFile(obj_path);
	model->createBuffer(device, physicalDevice, commandPool, queue);
	model->createDescriptorPool(device, FRAMES_IN_FLIGHT);
	model->createTexture(device, physicalDevice, commandPool, queue, texture_path);
	model->createUniformBuffer(device, physicalDevice, FRAMES_IN_FLIGHT);
	models.push_back(std::move(model));
}

void ModelManager::createModelDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT) {
	for (auto& model : models) {
		model->createDescriptorSet(device, descriptorSetLayout, FRAMES_IN_FLIGHT);
	}
}

void ModelManager::drawAll(VkCommandBuffer commandbuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame) {
	for (auto& model : models) {
		model->draw(commandbuffer, pipelineLayout, currentFrame);
	}
}

void ModelManager::cleanUp(VkDevice device) {
	for (auto& model : models) {
		model->cleanup(device);
	}
	models.clear();
}

Model* ModelManager::getModel(size_t index) { 
	if (index >= models.size()) return nullptr;
	return models[index].get(); 
}

std::vector<std::unique_ptr<Model>>& ModelManager::getModelList() {
	return models;
}