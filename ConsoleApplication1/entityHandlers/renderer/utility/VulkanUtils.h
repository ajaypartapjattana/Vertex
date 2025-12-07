#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>

namespace VulkanUtils {
    void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkCommandPool commandPool, VkDevice device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue);
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device);
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VkDevice device, VkCommandPool commandPool);
    void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkCommandPool commandPool, VkDevice device, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createTextureImageView(VkDevice, VkImage textureImage, VkImageView& textureImageView);
    VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice, VkSampler& textureSampler, VkFilter filtering);
    bool hasStencilComponent(VkFormat format);
}