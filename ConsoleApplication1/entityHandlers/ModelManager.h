#pragma once

#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>
#include <thread>
#include <atomic>
#include <filesystem>
#include <iostream>

#include "model.h"
#include "commProtocols/threadCommProtocol.h"

class ModelManager {
public:
	ModelManager() : running(true), loaderThread(&ModelManager::loaderLoop, this){}
	~ModelManager() {
		running = false;
		if (loaderThread.joinable()) loaderThread.join();
	}

	//runtime-loading:
	void requestLoad(const std::string& objPath, const std::string& texPath) {
		loadRequests.push({ objPath, texPath });
	}
	void update(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);
	std::unique_ptr<Model> loadModelData(const std::string& obj_path, const std::string& texture_path);
	void createLoadedModel(Model& model, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);

	//static-loading:
	void gatherModelData(const std::string& obj_path, const std::string& texture_path);
	void updateLoadedModels(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT);

	void drawAll(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame);
	void cleanUp(VkDevice device);
	void destroyModel(Model* model, VkDevice device);
	Model* getModel(size_t index);
	std::vector<std::unique_ptr<Model>>& getModelList();
	void saveModelMeta();
	void loadModelMeta();

	std::unordered_set<Model*> selectedModels;

private:
	std::vector<std::unique_ptr<Model>> models;

	struct LoadRequest {
		std::string object_path;
		std::string texture_path;
	};

	ThreadSafeQueue<LoadRequest> loadRequests;
	ThreadSafeQueue<std::unique_ptr<Model>> loadedModels;

	std::atomic<bool> running;
	std::thread loaderThread;

	void loaderLoop();
	
};