#pragma once

#include <vulkan/vulkan.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "core/Memory/memory.h"
#include "core/renderer_core.h"

#include "stdErr.h"

class Renderer {
private:
	mem::stack stash;

	VkInstance instance;

	struct PhysicalDevice {
		VkPhysicalDeviceProperties properties{};
		VkPhysicalDeviceMemoryProperties memory{};
		VkPhysicalDeviceFeatures features{};

		VkPhysicalDevice handle;
	};

	mem::span<PhysicalDevice> physicalDevices;
	PhysicalDevice* physicalDevice;

	mem::marker deployBase;

	VkSurfaceKHR surface;

	VkDevice device;

	struct QueueFamilyIndices {
		uint32_t graphics;
		uint32_t transfer;
		uint32_t present;
	} familyIndex;

	struct DeviceQueues {
		VkQueue graphics;
		VkQueue transfer;
		VkQueue present;
	} deviceQueue;

	VkCommandPool graphicsCommandPool;
	VkCommandPool transferCommandPool;

	VmaAllocator allocator;

	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	rndr::TransferStage transferStage;

	VkSwapchainKHR swapchain;
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	mem::span<VkImage> swapchainImage;
	mem::span<VkImageView> swapchainImageView;

	struct RenderPasses {
		VkRenderPass composite = VK_NULL_HANDLE;
	} renderPass;

	struct Samplers {
		VkSampler raw = VK_NULL_HANDLE;
	} sampler;

	struct DescriptorSetlayouts {
		VkDescriptorSetLayout imageRead = VK_NULL_HANDLE;
	} descriptorSetLayout;

	struct PipelineLayouts {
		VkPipelineLayout composite = VK_NULL_HANDLE;
	} pipelineLayout;

	struct Pipelines {
		VkPipeline composite = VK_NULL_HANDLE;
	} pipeline;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore transferCompleteSemaphore;
	
	VkFence fence;

	mem::marker deviceBase;

public:
	Renderer() noexcept = default;
	~Renderer() noexcept;

	int deploy() noexcept;

	void enumeratePhysicalDeviceNames(size_t* pCount, const char** const pDeviceName) noexcept;
	int createDevice(HINSTANCE hinstance, HWND hwnd, size_t physicalDeviceIndex);

	int sustainImage(const void* const pData, size_t _Size, uint32_t _Width, uint32_t _Height) noexcept;

	int createImage(const void* const pData, size_t _Size, uint32_t _Width, uint32_t _Height) noexcept;

	void reset() noexcept;

};