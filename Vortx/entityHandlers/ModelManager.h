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

#include "renderer/VulkanContext.h"

class ModelManager {
public:
	ModelManager(ContextHandle handle);
	~ModelManager();

	//runtime-loading:
	void requestLoad(const std::string& objPath, const std::string& texPath) {
		loadRequests.push({ objPath, texPath });
	}
	void update();
	std::unique_ptr<Model> loadModelData(const std::string& obj_path, const std::string& texture_path);
	void uploadModelToGPU(Model& model);

	//static-loading:
	void gatherModelData(const std::string& obj_path, const std::string& texture_path);
	void pushModelsToGPU();

	void drawAll(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame);
	void cleanUp();
	void destroyModel(Model* model);
	Model* getModel(size_t index);
	std::vector<std::unique_ptr<Model>>& getModelList();

	//model_config:
	void saveModelMeta();
	void loadModelMeta();

	std::unordered_set<Model*> selectedModels;

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue queue;
	VkCommandPool commandPool;

	VkDescriptorSetLayout descriptorSetLayout;
	uint16_t FRAMES_IN_FLIGHT;

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