#include "VulkanContext.h"

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, debugMessenger, pAllocator);
    }
}

VulkanContext::VulkanContext(GLFWwindow* window) : window(window) {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createCommandPool();
    createDepthResources();
    createMSAAResources();
    createFramebuffers();
    createCommandBuffers();
    createSyncObjects();
}

VulkanContext::~VulkanContext() {
    VulkanUtils::destroyImageResources(device, depthImage);
    VulkanUtils::destroyImageResources(device, MSAAImage);

    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto imageview : swapChainImageViews) {
        vkDestroyImageView(device, imageview, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    //vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMesseneger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

ContextHandle VulkanContext::getContext() {
    ContextHandle context{};
    context.instance = instance;

    context.device = device;
    context.physicalDevice = physicalDevice;
    context.graphicsQueue = graphicsQueue;
    context.presentQueue = presentQueue;
    context.commandPool = commandPool;
    context.descriptorSetLayout = descriptorSetLayout;

    context.renderPass = renderPass;

    context.MAX_FRAMES_IN_FLIGHT = MAX_FRAMES_IN_FLIGHT;

    return context;
}

void VulkanContext::recreateSwapChain() {
    int widht, height = 0;
    glfwGetFramebufferSize(window, &widht, &height);
    while (widht == 0 || height == 0) {
        glfwGetFramebufferSize(window, &widht, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device);

    cleanupSwapChain();
    imagesInFlight.clear();
    createSwapChain();

    createImageViews();
    createDepthResources();
    createMSAAResources();
    createFramebuffers();
}

void VulkanContext::cleanupSwapChain() {
    VulkanUtils::destroyImageResources(device, depthImage);
    VulkanUtils::destroyImageResources(device, MSAAImage);

    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto imageview : swapChainImageViews) {
        vkDestroyImageView(device, imageview, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice physicalDevice) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport && !indices.presentFamily.has_value()) {
            indices.presentFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}