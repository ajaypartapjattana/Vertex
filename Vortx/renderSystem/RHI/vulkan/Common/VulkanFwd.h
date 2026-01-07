#pragma once

#include <cstdint>

/* ============================================================
   Core Vulkan base types
   ============================================================ */

//typedef uint32_t VkFlags;
//typedef uint64_t VkDeviceSize;

/* ============================================================
   Dispatchable handles
   ============================================================ */

typedef struct VkInstance_T* VkInstance;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkCommandPool_T* VkCommandPool;

/* ============================================================
   Non-dispatchable handles
   ============================================================ */

typedef struct VkDeviceMemory_T* VkDeviceMemory;
typedef struct VkBuffer_T* VkBuffer;
typedef struct VkImage_T* VkImage;
typedef struct VkImageView_T* VkImageView;
typedef struct VkSampler_T* VkSampler;

typedef struct VkSemaphore_T* VkSemaphore;
typedef struct VkFence_T* VkFence;

typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;
typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkDescriptorSet_T* VkDescriptorSet;

typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkPipelineCache_T* VkPipelineCache;
typedef struct VkPipeline_T* VkPipeline;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkFramebuffer_T* VkFramebuffer;

typedef struct VkSwapchainKHR_T* VkSwapchainKHR;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;

typedef struct VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;

/* ============================================================
   Struct forward declarations (opaque usage only)
   ============================================================ */

//typedef struct VkExtent2D VkExtent2D;

/* ============================================================
   Enums (forward-declared)
   ============================================================ */

//typedef enum VkFormat VkFormat;
//typedef enum VkImageLayout VkImageLayout;
//typedef enum VkDescriptorType VkDescriptorType;
//typedef enum VkFilter VkFilter;

/* ============================================================
   Flag bits + flag masks
   ============================================================ */

//typedef enum VkShaderStageFlagBits VkShaderStageFlagBits;
//typedef VkFlags VkShaderStageFlags;
//
//typedef enum VkPipelineStageFlagBits VkPipelineStageFlagBits;
//typedef VkFlags VkPipelineStageFlags;
//
//typedef enum VkAccessFlagBits VkAccessFlagBits;
//typedef VkFlags VkAccessFlags;
//
//typedef enum VkImageAspectFlagBits VkImageAspectFlagBits;
//typedef VkFlags VkImageAspectFlags;
//
//typedef enum VkBufferUsageFlagBits VkBufferUsageFlagBits;
//typedef VkFlags VkBufferUsageFlags;
//
//typedef enum VkMemoryPropertyFlagBits VkMemoryPropertyFlagBits;
//typedef VkFlags VkMemoryPropertyFlags;

/* ============================================================
   Descriptor helper structs (only pointers allowed in headers)
   ============================================================ */

typedef struct VkDescriptorBufferInfo VkDescriptorBufferInfo;
typedef struct VkDescriptorImageInfo VkDescriptorImageInfo;
typedef struct VkWriteDescriptorSet VkWriteDescriptorSet;
