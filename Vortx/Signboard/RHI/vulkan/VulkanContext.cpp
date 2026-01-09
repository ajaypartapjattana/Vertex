#include "VulkanContext.h"

#include "Common/VulkanCommon.h"
#include "VulkanDevice.h"

#include <GLFW/glfw3.h>

VulkanContext::VulkanContext(GLFWwindow* window)
	: enableValidationLayers(DEBUG_MODE)
{
	configureExtensionsAndLayers();
	createInstance();
	setupDebugMessenger();
}

VulkanContext::~VulkanContext() {
	destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	if (instance) {
		vkDestroyInstance(instance, nullptr);
	}
}

void VulkanContext::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
		throw std::runtime_error("validation layers requested are not supported!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "MyEngine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "MyEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vulkan instance!");
	}
}

bool VulkanContext::checkValidationLayerSupport(
	const std::vector<const char*>&					layers) 
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : layers) {
		bool found = false;
		for (const auto& layer : availableLayers) {
			if (strcmp(layerName, layer.layerName) == 0) {
				found = true;
				break;
			}
		}
		if (!found)
			return false;
	}
	return true;
}

void VulkanContext::configureExtensionsAndLayers() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	if (!glfwExtensions) {
		throw std::runtime_error("GLFW did not returned required vulkan extensions!");
	}

	for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers) {
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT			severity,
	VkDebugUtilsMessageTypeFlagsEXT					type,
	const VkDebugUtilsMessengerCallbackDataEXT*		data,
	void*											userData)
{
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		fprintf(stderr, "Vulkan validation: %s\n", data->pMessage);
	}
	return VK_FALSE;
}

void VulkanContext::setupDebugMessenger() {
	if (!enableValidationLayers) return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
	if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to create debug messenger!");
	}
}

VkResult createDebugUtilsMessengerEXT(
	VkInstance										instance,
	const VkDebugUtilsMessengerCreateInfoEXT*		createInfo,
	const VkAllocationCallbacks*					allocator,
	VkDebugUtilsMessengerEXT*						messenger) 
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func) {
		return func(instance, createInfo, allocator, messenger);
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroyDebugUtilsMessengerEXT(
	VkInstance										instance,
	VkDebugUtilsMessengerEXT						messenger,
	const VkAllocationCallbacks*					allocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) {
		return func(instance, messenger, allocator);
	}
}
