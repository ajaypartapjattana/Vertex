#pragma once

#include "Common/VulkanFwd.h"

#include <vector>

#ifdef NDEBUG
const bool DEBUG_MODE = false;
#else
const bool DEBUG_MODE = true;
#endif

struct GLFWwindow;

class VulkanDevice;
class VulkanSwapchain;

class VulkanContext {
public:
	VulkanContext(GLFWwindow* window);
	~VulkanContext();

	VkInstance getInstance() const { return instance; }
	bool validationEnabled() const { return enableValidationLayers; }

private:
	void createInstance();
	void setupDebugMessenger();

	void configureExtensionsAndLayers();
	bool checkValidationLayerSupport(const std::vector<const char*>& layers);

private:
	VkInstance instance = nullptr;

	bool enableValidationLayers;
	VkDebugUtilsMessengerEXT debugMessenger = nullptr;

	std::vector<const char*> instanceExtensions;
	std::vector<const char*> validationLayers;
};