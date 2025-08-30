#include "ModelManager.h"

void ModelManager::loadModel(const std::string& path, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue) {
	auto model = std::make_unique<Model>();
	model->loadFromFile(path);
	model->createBuffer(device, physicalDevice, commandPool, queue);
	models.push_back(std::move(model));
}

void ModelManager::drawAll(VkCommandBuffer commandbuffer) {
	for (auto& model : models) {
		model->draw(commandbuffer);
	}
}

void ModelManager::cleanUp(VkDevice device) {
	for (auto& model : models) {
		model->cleanup(device);
	}
	models.clear();
}