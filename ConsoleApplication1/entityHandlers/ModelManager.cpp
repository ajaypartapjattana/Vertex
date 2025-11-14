#include "ModelManager.h"

void ModelManager::update(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT) {
	auto maybeModel = loadedModels.try_pop();
	while (maybeModel.has_value()) {
		auto model = std::move(maybeModel.value());
		createLoadedModel(*model, device, physicalDevice, commandPool, queue, descriptorSetLayout, FRAMES_IN_FLIGHT);
		models.push_back(std::move(model));

		maybeModel = loadedModels.try_pop();
	}
}

std::unique_ptr<Model> ModelManager::loadModelData(const std::string& obj_path, const std::string& texture_path) {
	auto model = std::make_unique<Model>();
	model->loadFromFile(obj_path);
	model->obj_path = obj_path;
	model->texture_path = texture_path;
	return model;
}

void ModelManager::createLoadedModel(Model& model, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT) {
	model.createBuffer(device, physicalDevice, commandPool, queue);
	model.createDescriptorPool(device, FRAMES_IN_FLIGHT);
	model.createTexture(device, physicalDevice, commandPool, queue, model.texture_path);
	model.createUniformBuffer(device, physicalDevice, FRAMES_IN_FLIGHT);
	model.createDescriptorSet(device, descriptorSetLayout, FRAMES_IN_FLIGHT);
}

void ModelManager::gatherModelData(const std::string& obj_path, const std::string& texture_path) {
	models.push_back(std::move(loadModelData(obj_path, texture_path)));
}

void ModelManager::updateLoadedModels(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT) {
	for (auto& model : models) {
		createLoadedModel(*model, device, physicalDevice, commandPool, queue, descriptorSetLayout, FRAMES_IN_FLIGHT);
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
	selectedModels.clear();
	models.clear();
}

void ModelManager::destroyModel(Model* model, VkDevice device) {
	if (!model) return;
	vkDeviceWaitIdle(device);
	model->cleanup(device);
	models.erase(std::remove_if(models.begin(), models.end(), [&](const std::unique_ptr<Model>& m) {return m.get() == model; }), models.end());
}

Model* ModelManager::getModel(size_t index) { 
	if (index >= models.size()) return nullptr;
	return models[index].get(); 
}

std::vector<std::unique_ptr<Model>>& ModelManager::getModelList() {
	return models;
}

void ModelManager::saveModelMeta() {
	json jAll = json::array();

	for (auto& model : models) {
		jAll.push_back(model->toJson());
	}

	std::ofstream file("models/models.json");
	file << jAll.dump(4);
}

void ModelManager::loadModelMeta() {
	std::ifstream file("models/models.json");
	if (!file.is_open()) {
		throw std::runtime_error("failed to gather metadata!");
	}
	json jall;
	file >> jall;

	models.clear();

	for (const auto& jModel : jall) {
		auto model = std::make_unique<Model>();
		model->fromJson(jModel);
		models.push_back(std::move(model));
	}
}

void ModelManager::loaderLoop() {
	while (running) {
		auto req = loadRequests.try_pop();
		if (!req.has_value()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		auto& [objPath, texPath] = req.value();
		std::cout << "Loading model on worker thread: " << objPath << std::endl;

		loadedModels.push(std::move(loadModelData(objPath, texPath)));

		std::cout << "[Loader] finished: " << objPath << std::endl;
	}
}

//added multithreaded model loader