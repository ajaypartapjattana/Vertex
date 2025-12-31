#pragma once

#include "Common/VulkanFwd.h"

#include <vector>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct GLFWwindow;

class VulkanDevice;
class VulkanSwapchain;

class VulkanContext {
public:
	VulkanContext(GLFWwindow* window);
	~VulkanContext();

	VkInstance getInstance() const { return instance; }
	VulkanDevice& getDevice() const { return *device; }
	bool validationEnabled() const { return validationEnabled; }

private:
	void createInstance();
	void setupDebugMessenger();

	void configureExtensionsAndLayers();
	bool checkValidationLayerSupport(const std::vector<const char*>& layers);

private:
	VkInstance instance = nullptr;

	bool validationEnabled;
	VkDebugUtilsMessengerEXT debugMessenger = nullptr;

	VulkanDevice* device = nullptr;

	std::vector<const char*> instanceExtensions;
	std::vector<const char*> validationLayers;
};