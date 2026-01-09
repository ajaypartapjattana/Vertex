#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <set>

#include "dataDef/ContextTypes.h"

class VulkanContext {
public:
    VulkanContext(GLFWwindow* window);
    ~VulkanContext();

    ContextHandle getContext();

    void recreateSwapChain();
    void cleanupSwapChain();

    VkCommandPool commandPool;

private:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMesseneger;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    GLFWwindow* window;

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    void createLogicalDevice();
    void createCommandPool();

private:

    std::vector<const char*> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }
    void setupDebugMessenger();
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
};