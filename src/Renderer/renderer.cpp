#include <array>
#include <algorithm>
#include <vector>

#if defined(WINDOW_WIN32)
	#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(WINDOW_XCB)
	#define VK_USE_PLATFORM_XCB_KHR
#elif defined(WINDOW_WAYLAND)
	#define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined(WINDOW_X11)
	#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.h>

#include <core/io/io.h>
#include <core/util.h>

#include "Vertex/vertex.hh"
#include "transfer.h"
#include "renderer.h"

constexpr uint32_t INVALID_QUEUE_FAMILY = UINT32_MAX;

struct PhysicalDevice {
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memory;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDevice handle;
};

struct VulkanContext_T {
	VkInstance instance;
	mem::span<PhysicalDevice> physicalDevices;
};

int requestVulkanContext(mem::stack* const pScratch, VulkanContext* const pContext) noexcept {
	VkInstance _instance = VK_NULL_HANDLE;
	
	do {
		VkResult result;

		pScratch->mark();
		
	#if defined(_DEBUG)
		constexpr std::array<const char*, 1> instanceLayers{
			"VK_LAYER_KHRONOS_validation"
		};
	#else
		constexpr std::array<const char*, 0> instanceLayers{

		};
	#endif

	#if defined(WINDOW_WIN32)
	  #if defined(_DEBUG)
		constexpr std::array<const char*, 3> instanceExtensions{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
			"VK_EXT_debug_utils"
		};
	  #else
		constexpr std::array<const char*, 2> instanceExtensions{
			"VK_KHR_surface",
			"VK_KHR_win32_surface"
		};
	  #endif
	#elif defined(WINDOW_XCB)
	  #if defined(_DEBUG)
		constexpr std::array<const char*, 3> instanceExtensions{
			"VK_KHR_surface",
			"VK_KHR_xcb_surface",
			"VK_EXT_debug_utils"
		};
	  #else
		constexpr std::array<const char*, 2> instanceExtensions{
			"VK_KHR_surface",
			"VK_KHR_xcb_surface"
		};
	  #endif
	#elif defined(WINDOW_WAYLAND)
	  #if defined(_DEBUG)
		constexpr std::array<const char*, 3> instanceExtensions{
			"VK_KHR_surface",
			"VK_KHR_wayland_surface",
			"VK_EXT_debug_utils"
		};
	  #else
		constexpr std::array<const char*, 2> instanceExtensions{
			"VK_KHR_surface",
			"VK_KHR_wayland_surface"
		};
	  #endif
	#elif defined(WINDOW_X11)
	  #if defined(_DEBUG)
		constexpr std::array<const char*, 3> instanceExtensions{
			"VK_KHR_surface",
			"VK_KHR_xlib_surface",
			"VK_EXT_debug_utils"
		};
	  #else
		constexpr std::array<const char*, 2> instanceExtensions{
			"VK_KHR_surface",
			"VK_KHR_xlib_surface"
		};
	  #endif
	#endif

		{
			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pNext = nullptr;
			appInfo.pApplicationName = "My Application";
			appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
			appInfo.pEngineName = "My Engine";
			appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_3;

			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
			createInfo.ppEnabledLayerNames = instanceLayers.data();
			createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
			createInfo.ppEnabledExtensionNames = instanceExtensions.data();

			result = vkCreateInstance(&createInfo, nullptr, &_instance);
		}

		if (result != VK_SUCCESS)
			break;

		uint32_t physicalDeviceCount;

		result = vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);

		if (result != VK_SUCCESS)
			break;

		mem::span<PhysicalDevice> physicalDevices = { new(std::nothrow) PhysicalDevice[physicalDeviceCount], (size_t)physicalDeviceCount };

		if (!physicalDevices)
			break;

		mem::span<VkPhysicalDevice> devicehandles = pScratch->alloc<VkPhysicalDevice>(physicalDeviceCount);

		if (!devicehandles)
			break;

		result = vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, devicehandles);
		
		if (result != VK_SUCCESS)
			break;

		for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
			const VkPhysicalDevice handle = devicehandles[i];

			vkGetPhysicalDeviceProperties(handle, &physicalDevices[i].properties);
			vkGetPhysicalDeviceMemoryProperties(handle, &physicalDevices[i].memory);
			vkGetPhysicalDeviceFeatures(handle, &physicalDevices[i].features);

			physicalDevices[i].handle = handle;
		}

		VulkanContext const context = new(std::nothrow) VulkanContext_T;
		
		if (!context)
		break;
		
		pScratch->restore();

		context->instance = _instance;
		context->physicalDevices = physicalDevices;
		
		*pContext = context;
		
		return 0;

	} while (false);

	pScratch->restore();

	if (_instance)
		vkDestroyInstance(_instance, nullptr);

	return -1;
}

void destroyVulkanContext(VulkanContext const _Context) noexcept {
	vkDestroyInstance(_Context->instance, nullptr);

	delete[] _Context->physicalDevices;
	delete _Context;
}

int enumeratePhysicalDevices(VulkanContext const _Context, uint32_t* const pCount, const char** const pDeviceNames) noexcept {
	assert(pCount);
	
	if (!pDeviceNames) {
		*pCount = static_cast<uint32_t>(_Context->physicalDevices.size());
		return 0;
	}

	const char** pDeviceName = pDeviceNames;

	const PhysicalDevice* const pPhysicalDeviceEnd = _Context->physicalDevices.pBegin + *pCount;
	for(const PhysicalDevice* pDevice{ _Context->physicalDevices.pBegin }; pDevice != pPhysicalDeviceEnd; ++pDevice) {
		*pDeviceName++ = pDevice->properties.deviceName;
	}

	return 0;
}

constexpr uint32_t MIN_BUFFERING_COUNT = 2;

struct QueueFamilyIndices {
	uint32_t graphics;
	uint32_t transfer;
	uint32_t present;
};

struct DeviceQueues {
	VkQueue graphics;
	VkQueue transfer;
	VkQueue present;
};

struct CommandPools {
	VkCommandPool graphics;
	VkCommandPool transfer;
};

struct RenderPasses {
	VkRenderPass composite;
};

struct Samplers {
	VkSampler raw;
};

struct DescriptorSetlayouts {
	VkDescriptorSetLayout imageRead;
};

struct PipelineLayouts {
	VkPipelineLayout composite;
};

struct Pipelines {
	VkPipeline composite;
};

struct Semaphores {
	VkSemaphore imageAvailable;
	VkSemaphore transferComplete;
};

struct Emulator_T {
	VkInstance instance;
	VkPhysicalDevice physicalDevice;

	VkSurfaceKHR surface;
	VkDevice device;
	QueueFamilyIndices queueFamily;
	DeviceQueues queue;
	
};

int createEmulator(VulkanContext const _Context, const EmulatorCreateInfo* const pCreateInfo, mem::stack* const pScratch, Emulator* const pEmulator) noexcept {
	const VkInstance instance = _Context->instance;
	
	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;

	do {
		VkResult result;

		pScratch->mark();
		
	  #if defined(WINDOW_WIN32)
		{
			VkWin32SurfaceCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.connection = reinterpret_cast<HINSTANCE>(pCreateInfo->windowContext);
			createInfo.window = static_cast<HWND>(pCreateInfo->windowHandle);

			result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &_surface);
		}
	  #elif defined(WINDOW_XCB)
		{
			VkXcbSurfaceCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.connection = reinterpret_cast<xcb_connection_t*>(pCreateInfo->windowContext);
			createInfo.window = static_cast<xcb_window_t>(pCreateInfo->windowHandle);

			result = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &_surface);
		}
	  #elif defined(WINDOW_WAYLAND)
	  	{
			VkXcbSurfaceCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.connection = reinterpret_cast<xcb_connection_t*>(pCreateInfo->windowContext);
			createInfo.window = static_cast<xcb_window_t>(pCreateInfo->windowHandle);

			result = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &_surface);
		}
	  #elif defined(WINDOW_X11)
	  	{
			VkXcbSurfaceCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.connection = reinterpret_cast<xcb_connection_t*>(pCreateInfo->windowContext);
			createInfo.window = static_cast<xcb_window_t>(pCreateInfo->windowHandle);

			result = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &_surface);
		}
	  #endif

		if (result != VK_SUCCESS)
			break;

		const uint32_t physicalDeviceIndex = pCreateInfo->physicalDevice;
		const VkPhysicalDevice physicalDevice = _Context->physicalDevices[physicalDeviceIndex].handle;

		mem::static_vector<uint32_t> uniqueQueueFamilies = pScratch->alloc<uint32_t>(3u);

		QueueFamilyIndices queueFamily = { INVALID_QUEUE_FAMILY, INVALID_QUEUE_FAMILY, INVALID_QUEUE_FAMILY };

		{
			uint32_t queueFamilyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

			mem::span<VkQueueFamilyProperties> queueFamilies = pScratch->alloc<VkQueueFamilyProperties>(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

			mem::span<uint32_t> familyRole = pScratch->alloc<uint32_t>(queueFamilyCount);
			familyRole.assign_default();

			constexpr VkQueueFlags releventCapabilities = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

			for (uint32_t i = 0; i < queueFamilyCount; ++i) {
				VkQueueFlags queueFlags = queueFamilies[i].queueFlags & releventCapabilities;
				familyRole[i] = intrin_popCount32(queueFlags);
			}

			uint32_t min = UINT32_MAX;

			for (uint32_t i = 0; i < queueFamilyCount; ++i) {
				if((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
					continue;

				if (familyRole[i] >= min)
					continue;

				min = familyRole[i];
				queueFamily.graphics = i;
				uniqueQueueFamilies.push_back_unique(i);
			}

			min = UINT32_MAX;

			for (uint32_t i = 0; i < queueFamilyCount; ++i) {
				if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0)
					continue;

				if (familyRole[i] >= min)
					continue;

				min = familyRole[i];
				queueFamily.transfer = i;
				uniqueQueueFamilies.push_back_unique(i);
			}

			VkBool32 presentSupport = VK_FALSE;

			for (uint32_t i = 0; i < queueFamilyCount; ++i) {
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surface, &presentSupport);
				if (!presentSupport)
					continue;
					
				queueFamily.present = i;
				uniqueQueueFamilies.push_back_unique(i);

				break;
			}
		}

		if (uniqueQueueFamilies.empty())
			break;
			
		{
			const size_t queueInfoCount = uniqueQueueFamilies.size();

			mem::span<VkDeviceQueueCreateInfo> queueInfos = pScratch->alloc<VkDeviceQueueCreateInfo>(queueInfoCount);

			constexpr float queuePriority = 1.0f;

			for (size_t i = 0; i < queueInfoCount; ++i) {
				VkDeviceQueueCreateInfo* pInfo = &queueInfos[i];

				pInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				pInfo->pNext = nullptr;
				pInfo->flags = 0;
				pInfo->queueFamilyIndex = uniqueQueueFamilies[i];
				pInfo->queueCount = 1;
				pInfo->pQueuePriorities = &queuePriority;
			}

			constexpr std::array<const char*, 1> extensions{
				"VK_KHR_swapchain"
			};

			VkPhysicalDeviceFeatures enabledFeatures{};

			if (_Context->physicalDevices[physicalDeviceIndex].features.samplerAnisotropy)
				enabledFeatures.samplerAnisotropy = VK_TRUE;

			VkDeviceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
			createInfo.pQueueCreateInfos = queueInfos;
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();
			createInfo.pEnabledFeatures = &enabledFeatures;

			result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &_device);
		}

		if (result != VK_SUCCESS)
			break;

		DeviceQueues deviceQueue;

		vkGetDeviceQueue(_device, queueFamily.graphics, 0, &deviceQueue.graphics);
		vkGetDeviceQueue(_device, queueFamily.transfer, 0, &deviceQueue.transfer);
		vkGetDeviceQueue(_device, queueFamily.present, 0, &deviceQueue.present);

		Emulator const emulator = new(std::nothrow) Emulator_T;

		if (!emulator)
			break;

		pScratch->restore();

		emulator->instance = instance;
		emulator->physicalDevice = physicalDevice;

		emulator->surface = _surface;
		emulator->device = _device;
		emulator->queueFamily = queueFamily;
		emulator->queue = deviceQueue;

		*pEmulator = emulator;

		return 0;

	} while (false);

	pScratch->restore();

	if (_device)
		vkDestroyDevice(_device, nullptr);

	if (_surface)
		vkDestroySurfaceKHR(_Context->instance, _surface, nullptr);

	return -1;
}

void destroyEmulator(Emulator const _Emulator) noexcept {
	vkDestroyDevice(_Emulator->device, nullptr);

	vkDestroySurfaceKHR(_Emulator->instance, _Emulator->surface, nullptr);

	delete _Emulator;
}

int waitEmulator(Emulator const _Emulator) noexcept {
	const VkDevice device = _Emulator->device;

	if (vkDeviceWaitIdle(device) == VK_SUCCESS)
		return 0;

	return -1;
}

struct Image {
	VkImage image;
	VmaAllocation allocation;
};

struct Buffer {
	VkBuffer buffer;
	VmaAllocation allocation;
};

struct AsyncLoader_T {
	VkDevice device;
	VmaAllocator allocator;
	VkCommandPool commandPool;
	mem::span<VkCommandBuffer> commandBuffer;
	mem::span<VkSemaphore> semaphore;
	mem::span<VkFence> fence;
	Buffer stageBuffer;
	rndr::TransferStage stage;
};

int createAsyncLoader(Emulator const _Emulator, const AsyncLoaderCreateInfo* const pCreateInfo, AsyncLoader* const pAsyncLoader) noexcept {
	const VkDevice device = _Emulator->device;
	
	VmaAllocator _allocator = VK_NULL_HANDLE;
	VkCommandPool _commandPool = VK_NULL_HANDLE;
	mem::span<VkCommandBuffer> _commandBuffer;
	mem::span<VkSemaphore> _sempahore;
	mem::span<VkFence> _fence;
	Buffer _stageBuffer{};

	do {
		VkResult result;

		{
			VmaAllocatorCreateInfo createInfo{};
			createInfo.flags = 0;
			createInfo.physicalDevice = _Emulator->physicalDevice;
			createInfo.device = device;
			createInfo.preferredLargeHeapBlockSize = 0;
			createInfo.pAllocationCallbacks = nullptr;
			createInfo.pDeviceMemoryCallbacks = nullptr;
			createInfo.pHeapSizeLimit = nullptr;
			createInfo.pVulkanFunctions = nullptr;
			createInfo.instance = _Emulator->instance;
			createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
			createInfo.pTypeExternalMemoryHandleTypes = nullptr;

			result = vmaCreateAllocator(&createInfo, &_allocator);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkCommandPoolCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			createInfo.queueFamilyIndex = _Emulator->queueFamily.transfer;

			result = vkCreateCommandPool(device, &createInfo, nullptr, &_commandPool);
		}

		if (result != VK_SUCCESS)
			break;

		_commandBuffer = { new(std::nothrow) VkCommandBuffer[pCreateInfo->maxAsyncLoadRate], (size_t)pCreateInfo->maxAsyncLoadRate };

		if (!_commandBuffer)
			break;

		{
			VkCommandBufferAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocateInfo.pNext = nullptr;
			allocateInfo.commandPool = _commandPool;
			allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocateInfo.commandBufferCount = pCreateInfo->maxAsyncLoadRate;
			
			result = vkAllocateCommandBuffers(device, &allocateInfo, _commandBuffer);
		}

		if (result != VK_SUCCESS)
			break;

		_sempahore = { new(std::nothrow) VkSemaphore[pCreateInfo->maxAsyncLoadRate], (size_t)pCreateInfo->maxAsyncLoadRate };

		_sempahore.assign_default();

		{
			VkSemaphoreCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;

			const VkSemaphore* const pSemaphoreEnd = _sempahore.pEnd;
			for(VkSemaphore* pSemaphore{ _sempahore.pBegin }; pSemaphore != pSemaphoreEnd && result == VK_SUCCESS; ++pSemaphore)
				result = vkCreateSemaphore(device, &createInfo, nullptr, pSemaphore);
		}

		if (result != VK_SUCCESS)
			break;

		_fence = { new(std::nothrow) VkFence[pCreateInfo->maxAsyncLoadRate], (size_t)pCreateInfo->maxAsyncLoadRate };

		if (!_fence)
			break;

		_fence.assign_default();

		{
			VkFenceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			const VkFence* const pFenceEnd = _fence.pEnd;
			for (VkFence* pFence{ _fence.pBegin }; pFence != pFenceEnd && result == VK_SUCCESS; ++pFence)
				result = vkCreateFence(device, &createInfo, nullptr, pFence);
		}

		if (result != VK_SUCCESS)
			break;

		mem::span<uint8_t> stagingSpan;
			
		{
			VkBufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.size = VkDeviceSize(pCreateInfo->stageSize);
			createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;

			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocationCreateInfo.requiredFlags = 0;
			allocationCreateInfo.preferredFlags = 0;
			allocationCreateInfo.memoryTypeBits = 0;
			allocationCreateInfo.pool = VK_NULL_HANDLE;
			allocationCreateInfo.pUserData = nullptr;
			allocationCreateInfo.priority = 0.0f;

			VmaAllocationInfo allocationInfo;
			result = vmaCreateBuffer(_allocator, &createInfo, &allocationCreateInfo, &_stageBuffer.buffer, &_stageBuffer.allocation, &allocationInfo);

			stagingSpan = mem::span<uint8_t>{ reinterpret_cast<uint8_t*>(allocationInfo.pMappedData), static_cast<size_t>(allocationInfo.size) };
		}

		if (result != VK_SUCCESS)
			break;
			
		AsyncLoader const loader = new(std::nothrow) AsyncLoader_T;

		if (!loader)
			break;

		loader->stage = { stagingSpan };
		loader->stageBuffer = _stageBuffer;
		loader->fence = _fence;
		loader->semaphore = _sempahore;
		loader->commandBuffer = _commandBuffer;
		loader->commandPool = _commandPool;
		loader->allocator = _allocator;
		loader->device = device;
		
		*pAsyncLoader = loader;

		return 0;

	} while (false);

	if (_stageBuffer.buffer)
		vmaDestroyBuffer(_allocator, _stageBuffer.buffer, _stageBuffer.allocation);

	if (_fence) {
		const VkFence* const pFenceEnd = _fence.pEnd;
		for (const VkFence* pFence{ _fence.pBegin }; pFence != pFenceEnd && *pFence; ++pFence)
			vkDestroyFence(device, *pFence, nullptr);

		delete[] _fence;
	}

	if (_sempahore) {
		const VkSemaphore* const pSemaphoreEnd = _sempahore.pEnd;
		for (const VkSemaphore* pSemaphore{ _sempahore.pBegin }; pSemaphore != pSemaphoreEnd && *pSemaphore; ++pSemaphore)
			vkDestroySemaphore(device, *pSemaphore, nullptr);

		delete[] _sempahore;
	}

	if (_commandBuffer)
		delete[] _commandBuffer;

	if (_commandPool)
		vkDestroyCommandPool(device, _commandPool, nullptr);

	if (_allocator)
		vmaDestroyAllocator(_allocator);

	return -1;
}

void destroyAsyncLoader(AsyncLoader const _AsynLoader) noexcept {
	vmaDestroyBuffer(_AsynLoader->allocator, _AsynLoader->stageBuffer.buffer, _AsynLoader->stageBuffer.allocation);

	const VkFence* const pFenceEnd = _AsynLoader->fence.pEnd;
	for (const VkFence* pFence{ _AsynLoader->fence.pBegin }; pFence != pFenceEnd; ++pFence)
		vkDestroyFence(_AsynLoader->device, *pFence, nullptr);

	delete[] _AsynLoader->fence;

	const VkSemaphore* const pSemaphoreEnd = _AsynLoader->semaphore.pEnd;
	for (const VkSemaphore* pSemaphore{ _AsynLoader->semaphore.pBegin }; pSemaphore != pSemaphoreEnd; ++pSemaphore)
		vkDestroySemaphore(_AsynLoader->device, *pSemaphore, nullptr);

	delete[] _AsynLoader->semaphore;

	delete[] _AsynLoader->commandBuffer;
	vkDestroyCommandPool(_AsynLoader->device, _AsynLoader->commandPool, nullptr);
	
	vmaDestroyAllocator(_AsynLoader->allocator);

	delete _AsynLoader;
}

struct Canvas_T {
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;

	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;

	mem::span<VkClearValue> clearValue;
	VkRenderPass renderPass;

	VkExtent2D extent;
	uint32_t layers;
	VkSwapchainKHR swapchain;
	mem::span<VkImage> image;
	mem::span<VkImageView> imageView;

	mem::span<VkFramebuffer> framebuffer;
	mem::span<VkFence> fence;
};

int createCanvas(Emulator const _Emulator, const CanvasCreateInfo* const pCreateInfo, mem::stack* const pScratch, Canvas* const pCanvas) noexcept {
	const VkSurfaceKHR surface = _Emulator->surface;
	const VkPhysicalDevice physicalDevice = _Emulator->physicalDevice;
	const VkDevice device = _Emulator->device;

	mem::span<VkClearValue> _clearValue;
	VkRenderPass _renderPass = VK_NULL_HANDLE;

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	mem::span<VkImage> _image;
	mem::span<VkImageView> _imageView;

	mem::span<VkFramebuffer> _framebuffer;
	mem::span<VkFence> _fence;

	do {
		VkResult result;

		pScratch->mark();

		VkSurfaceFormatKHR surfaceFormat;

		{
			constexpr VkSurfaceFormatKHR preferredSurfaceFormats[] {
				{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
				{ VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
				{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
				{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
			};

			uint32_t surfaceFormatCount;

			result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);

			if (result != VK_SUCCESS)
				break;

			mem::span<VkSurfaceFormatKHR> surfaceFormats = pScratch->alloc<VkSurfaceFormatKHR>(surfaceFormatCount);

			result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats);
				
			if (result != VK_SUCCESS)
				break;

			const VkSurfaceFormatKHR* pSurfaceFormat = nullptr;

			const VkSurfaceFormatKHR* const pFormatEnd = preferredSurfaceFormats + 4u;
			for (const VkSurfaceFormatKHR* pFormat{ preferredSurfaceFormats }; pFormat != pFormatEnd; ++pFormat) {
				pSurfaceFormat = std::find_if(surfaceFormats.pBegin, surfaceFormats.pEnd, [pFormat](const VkSurfaceFormatKHR& _Format) { return _Format.format == pFormat->format && _Format.colorSpace == pFormat->colorSpace; });

				if (pSurfaceFormat != surfaceFormats.pEnd)
					break;

				pSurfaceFormat = nullptr;
			}

			if (!pSurfaceFormat)
				break;

			surfaceFormat = *pSurfaceFormat;
		}

		VkPresentModeKHR presentMode;

		{
			uint32_t presentModeCount;

			result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

			if (result != VK_SUCCESS)
				break;

			mem::span<VkPresentModeKHR> presentModes = pScratch->alloc<VkPresentModeKHR>(presentModeCount);

			result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);

			if (result != VK_SUCCESS)
				break;

			VkPresentModeKHR* pPresentMode = std::find(presentModes.pBegin, presentModes.pEnd, VK_PRESENT_MODE_MAILBOX_KHR);

			presentMode = pPresentMode != presentModes.pEnd ? *pPresentMode : VK_PRESENT_MODE_FIFO_KHR;
		}

		_clearValue = { new(std::nothrow) VkClearValue[1u], (size_t)1u };

		if (!_clearValue)
			break;

		_clearValue[0] = {{{ 0.0f, 0.0f, 0.0f, 1.0f } }};
		
		{
			VkAttachmentDescription attachment{};
			attachment.format = surfaceFormat.format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference reference{};
			reference.attachment = 0;
			reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.flags = 0;
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.inputAttachmentCount = 0;
			subpass.pInputAttachments = nullptr;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &reference;
			subpass.pResolveAttachments = nullptr;
			subpass.pDepthStencilAttachment = nullptr;
			subpass.preserveAttachmentCount = 0;
			subpass.pPreserveAttachments = nullptr;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_NONE;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = 0;

			VkRenderPassCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = &attachment;
			createInfo.subpassCount = 1;
			createInfo.pSubpasses = &subpass;
			createInfo.dependencyCount = 1;
			createInfo.pDependencies = &dependency;

			result = vkCreateRenderPass(device, &createInfo, nullptr, &_renderPass);
		}

		if (result != VK_SUCCESS)
			break;


		VkSurfaceCapabilitiesKHR surfaceCapabilities;

		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

		if (result != VK_SUCCESS)
			break;

		uint32_t minImageCount = pCreateInfo->minImageCount;

		{
			minImageCount = std::max<uint32_t>(minImageCount, surfaceCapabilities.minImageCount);

			if (surfaceCapabilities.maxImageCount)
				minImageCount = std::min<uint32_t>(minImageCount, surfaceCapabilities.maxImageCount);
		}

		{
			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.surface = surface;
			createInfo.minImageCount = minImageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = surfaceCapabilities.currentExtent;
			createInfo.imageArrayLayers = 1u;
			createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
			createInfo.preTransform = surfaceCapabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &_swapchain);
		}

		if (result != VK_SUCCESS)
			break;

		uint32_t imageCount;
			
		result = vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, nullptr);

		if (result != VK_SUCCESS)
			break;

		_image = { new(std::nothrow) VkImage[imageCount], (size_t)imageCount };

		if (!_image)
			break;

		result = vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, _image);
			
		if (result != VK_SUCCESS)
			break;

		_imageView = { new(std::nothrow) VkImageView[imageCount], (size_t)imageCount };

		if (!_imageView)
			break;

		_imageView.assign_default();

		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = surfaceFormat.format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkImageView* pImageView = _imageView.pBegin;

			const VkImage* const pImageEnd = _image.pEnd;
			for (const VkImage* pImage{ _image.pBegin }; pImage != pImageEnd && result == VK_SUCCESS; ++pImage) {
				createInfo.image = *pImage;
				result = vkCreateImageView(device, &createInfo, nullptr, pImageView++);
			}
		}

		if (result != VK_SUCCESS)
			break;

		_framebuffer = { new(std::nothrow) VkFramebuffer[imageCount], (size_t)imageCount };

		if (!_framebuffer)
			break;

		_framebuffer.assign_default();

		{
			VkFramebufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.renderPass = _renderPass;
			createInfo.attachmentCount = 1u;
			createInfo.width = surfaceCapabilities.currentExtent.width;
			createInfo.height = surfaceCapabilities.currentExtent.height;
			createInfo.layers = 1u;

			const VkImageView* pImageView = _imageView.pBegin;

			const VkFramebuffer* const pFramebufferEnd = _framebuffer.pEnd;
			for (VkFramebuffer* pFramebuffer{ _framebuffer.pBegin }; pFramebuffer != pFramebufferEnd && result == VK_SUCCESS; ++pFramebuffer) {
				createInfo.pAttachments = pImageView++;

				result = vkCreateFramebuffer(device, &createInfo, nullptr, pFramebuffer);
			}
		}

		if (result != VK_SUCCESS)
			break;

		_fence = { new(std::nothrow) VkFence[imageCount], (size_t)imageCount };
		
		if (!_fence)
			break;

		_fence.assign_default();

		Canvas const canvas = new(std::nothrow) Canvas_T;

		if (!canvas)
			break;

		pScratch->restore();

		canvas->fence = _fence;
		canvas->framebuffer = _framebuffer;

		canvas->imageView = _imageView;
		canvas->image = _image;
		canvas->swapchain = _swapchain;
		canvas->layers = 1u;
		canvas->extent = surfaceCapabilities.currentExtent;
		
		canvas->renderPass = _renderPass;
		canvas->clearValue = _clearValue;

		canvas->presentMode = presentMode;
		canvas->surfaceFormat = surfaceFormat;

		canvas->device = device;
		canvas->physicalDevice = physicalDevice;
		canvas->surface = surface;

		*pCanvas = canvas;

		return 0;

	} while (false);

	pScratch->restore();

	if (_fence)
		delete[] _fence;

	if (_framebuffer) {
		const VkFramebuffer* const pFramebufferEnd = _framebuffer.pEnd;
		for (const VkFramebuffer* pFramebuffer{ _framebuffer.pBegin }; pFramebuffer != pFramebufferEnd && *pFramebuffer; ++pFramebuffer)
			vkDestroyFramebuffer(device, *pFramebuffer, nullptr);

		delete[] _framebuffer;
	}

	if (_imageView) {
		const VkImageView* const pImageViewEnd = _imageView.pEnd;
		for (const VkImageView* pImageView{ _imageView.pBegin }; pImageView != pImageViewEnd && *pImageView; ++pImageView)
			vkDestroyImageView(device, *pImageView, nullptr);

		delete[] _imageView;
	}

	if (_image)
		delete[] _image;

	if (_swapchain)
		vkDestroySwapchainKHR(device, _swapchain, nullptr);

	if (_renderPass)
		vkDestroyRenderPass(device, _renderPass, nullptr);

	if (_clearValue)
		delete[] _clearValue;

	return -1;
}

void destroyCanvas(Canvas const _Canvas) noexcept {
	const VkDevice device = _Canvas->device;

	delete[] _Canvas->fence;

	const VkFramebuffer* const pFramebufferEnd = _Canvas->framebuffer.pEnd;
	for (const VkFramebuffer* pFramebuffer{ _Canvas->framebuffer.pBegin }; pFramebuffer != pFramebufferEnd; ++pFramebuffer)
		vkDestroyFramebuffer(device, *pFramebuffer, nullptr);

	delete[] _Canvas->framebuffer;
	
	const VkImageView* const pImageViewEnd = _Canvas->imageView.pEnd;
	for (const VkImageView* pImageView{ _Canvas->imageView.pBegin }; pImageView != pImageViewEnd; ++pImageView)
		vkDestroyImageView(device, *pImageView, nullptr);

	delete[] _Canvas->imageView;
	delete[] _Canvas->image;

	vkDestroySwapchainKHR(device, _Canvas->swapchain, nullptr);

	vkDestroyRenderPass(device, _Canvas->renderPass, nullptr);
	delete[] _Canvas->clearValue;

	delete _Canvas;
}

int updateCanvas(Canvas const _Canvas) noexcept {
	const VkSurfaceKHR surface = _Canvas->surface;
	const VkPhysicalDevice physicalDevice = _Canvas->physicalDevice;
	const VkDevice device = _Canvas->device;

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	mem::span<VkImage> _image;
	mem::span<VkImageView> _imageView;

	mem::span<VkFramebuffer> _framebuffer;
	mem::span<VkFence> _fence;

	do {
		VkResult result;
		
		VkSurfaceCapabilitiesKHR surfaceCapabilities;

		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

		if (result != VK_SUCCESS)
			break;

		uint32_t minImageCount = static_cast<uint32_t>(_Canvas->image.size());

		{
			minImageCount = std::max<uint32_t>(minImageCount, surfaceCapabilities.minImageCount);

			if (surfaceCapabilities.maxImageCount)
				minImageCount = std::min<uint32_t>(minImageCount, surfaceCapabilities.maxImageCount);
		}

		{
			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.surface = surface;
			createInfo.minImageCount = minImageCount;
			createInfo.imageFormat = _Canvas->surfaceFormat.format;
			createInfo.imageColorSpace = _Canvas->surfaceFormat.colorSpace;
			createInfo.imageExtent = surfaceCapabilities.currentExtent;
			createInfo.imageArrayLayers = 1u;
			createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
			createInfo.preTransform = surfaceCapabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = _Canvas->presentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = _Canvas->swapchain;

			result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &_swapchain);
		}

		if (result != VK_SUCCESS)
			break;

		uint32_t imageCount;
		
		result = vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, nullptr);

		if (result != VK_SUCCESS)
			break;

		_image = { new(std::nothrow) VkImage[imageCount], (size_t)imageCount };

		if (!_image)
			break;

		result = vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, _image);

		if (result != VK_SUCCESS)
			break;

		_imageView = { new(std::nothrow) VkImageView[imageCount], (size_t)imageCount };

		if (!_imageView)
			break;

		_imageView.assign_default();

		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = _Canvas->surfaceFormat.format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkImageView* pImageView = _imageView;

			const VkImage* const pImageEnd = _image.pEnd;
			for (const VkImage* pImage{ _image.pBegin }; pImage != pImageEnd && result == VK_SUCCESS; ++pImage){
				createInfo.image = *pImage;
				result = vkCreateImageView(device, &createInfo, nullptr, pImageView++);
			}
		}

		if (result != VK_SUCCESS)
			break;

		_framebuffer = { new(std::nothrow) VkFramebuffer[imageCount], (size_t)imageCount };
		
		if (!_framebuffer)
			break;

		_framebuffer.assign_default();

		{
			VkFramebufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.renderPass = _Canvas->renderPass;
			createInfo.attachmentCount = 1u;
			createInfo.width = surfaceCapabilities.currentExtent.width;
			createInfo.height = surfaceCapabilities.currentExtent.height;
			createInfo.layers = 1u;

			const VkImageView* pImageView = _imageView.pBegin;

			const VkFramebuffer* const pFramebufferEnd = _framebuffer.pEnd;
			for (VkFramebuffer* pFramebuffer{ _framebuffer.pBegin }; pFramebuffer != pFramebufferEnd && result == VK_SUCCESS; ++pFramebuffer) {
				createInfo.pAttachments = pImageView++;

				result = vkCreateFramebuffer(device, &createInfo, nullptr, pFramebuffer);
			}
		}

		if (result != VK_SUCCESS)
			break;

		_fence = { new(std::nothrow) VkFence[imageCount], (size_t)imageCount };

		if (!_fence)
			break;

		_fence.assign_default();

		const VkFence* const pFenceEnd = _Canvas->fence.pEnd;
		for (const VkFence* pFence{ _Canvas->fence.pBegin }; pFence != pFenceEnd && result == VK_SUCCESS; ++pFence)
			result = *pFence ? vkWaitForFences(device, 1u, pFence, VK_TRUE, UINT64_MAX) : VK_SUCCESS;

		if (result != VK_SUCCESS)
			break;

		delete[] _Canvas->fence;

		const VkFramebuffer* const pFramebufferEnd = _Canvas->framebuffer.pEnd;
		for (const VkFramebuffer* pFramebuffer{ _Canvas->framebuffer.pBegin }; pFramebuffer != pFramebufferEnd; ++pFramebuffer)
			vkDestroyFramebuffer(device, *pFramebuffer, nullptr);

		delete[] _Canvas->framebuffer;
			
		const VkImageView* const pImageViewEnd = _Canvas->imageView.pEnd;
		for (const VkImageView* pImageView{ _Canvas->imageView.pBegin }; pImageView != pImageViewEnd; ++pImageView)
			vkDestroyImageView(device, *pImageView, nullptr);

		delete[] _Canvas->imageView;
		delete[] _Canvas->image;

		vkDestroySwapchainKHR(device, _Canvas->swapchain, nullptr);

		_Canvas->fence = _fence;
		_Canvas->framebuffer = _framebuffer;
		_Canvas->imageView = _imageView;
		_Canvas->image = _image;
		_Canvas->swapchain = _swapchain;
		_Canvas->layers = 1u;
		_Canvas->extent = surfaceCapabilities.currentExtent;

		return 0;

	} while (false);

	if (_fence)
		delete[] _fence;

	if (_framebuffer) {
		const VkFramebuffer* const pFramebufferEnd = _framebuffer.pEnd;
		for (const VkFramebuffer* pFramebuffer{ _framebuffer.pBegin }; pFramebuffer != pFramebufferEnd && *pFramebuffer; ++pFramebuffer)
			vkDestroyFramebuffer(device, *pFramebuffer, nullptr);

		delete[] _framebuffer;
	}

	if (_imageView) {
		const VkImageView* const pImageViewEnd = _imageView.pEnd;
		for (const VkImageView* pImageView{ _imageView.pBegin }; pImageView != pImageViewEnd && *pImageView; ++pImageView)
			vkDestroyImageView(device, *pImageView, nullptr);

		delete[] _imageView;
	}

	if (_image)
		delete[] _image;

	if (_swapchain)
		vkDestroySwapchainKHR(device, _swapchain, nullptr);

	return -1;
}

struct RenderBox_T {
	VkDevice device;

	VkSampler sampler;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
};

int createRenderBox(Emulator const _Emulator, Canvas const _Canvas, mem::stack* const pScratch, RenderBox* const pRenderBox) noexcept {
	const VkDevice device = _Emulator->device;

	VkSampler _sampler = VK_NULL_HANDLE;
	VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkPipeline _pipeline = VK_NULL_HANDLE;

	do {
		VkResult result;

		{
			VkSamplerCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.magFilter = VK_FILTER_LINEAR;
			createInfo.minFilter = VK_FILTER_LINEAR;
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			createInfo.mipLodBias = 1.0f;
			createInfo.anisotropyEnable = VK_FALSE;
			createInfo.maxAnisotropy = 1.0f;
			createInfo.compareEnable = VK_FALSE;
			createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			createInfo.minLod = 0.0f;
			createInfo.maxLod = VK_LOD_CLAMP_NONE;
			createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			createInfo.unnormalizedCoordinates = VK_FALSE;

			result = vkCreateSampler(device, &createInfo, nullptr, &_sampler);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = 0;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			binding.pImmutableSamplers = &_sampler;

			VkDescriptorSetLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.bindingCount = 1u;
			createInfo.pBindings = &binding;

			result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &_descriptorSetLayout);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkPipelineLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.setLayoutCount = 1;
			createInfo.pSetLayouts = &_descriptorSetLayout;
			createInfo.pushConstantRangeCount = 0;
			createInfo.pPushConstantRanges = nullptr;

			result = vkCreatePipelineLayout(device, &createInfo, nullptr, &_pipelineLayout);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkShaderModule vertex;

			{
				const char* const vertPath = "shaders/base.vert.spv";

				size_t size;

				int error = io::getBinarySize(vertPath, &size);

				if (error)
					break;

				pScratch->mark();

				mem::span<uint8_t> bin = pScratch->alloc<uint8_t>(size);

				error = io::loadBinary(vertPath, size, bin);

				if (error)
					break;

				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.pNext = nullptr;
				createInfo.flags = 0;
				createInfo.codeSize = size;
				createInfo.pCode = reinterpret_cast<const uint32_t*>(bin.pBegin);

				result = vkCreateShaderModule(device, &createInfo, nullptr, &vertex);

				pScratch->restore();
			}

			if (result != VK_SUCCESS)
				break;

			VkShaderModule fragment;

			{
				const char* const fragPath = "shaders/base.frag.spv";

				size_t size;

				int error = io::getBinarySize(fragPath, &size);

				if (error) {
					vkDestroyShaderModule(device, vertex, nullptr);
					break;
				}

				pScratch->mark();

				mem::span<uint8_t> bin = pScratch->alloc<uint8_t>(size);

				error = io::loadBinary(fragPath, size, bin);

				if (error) {
					vkDestroyShaderModule(device, vertex, nullptr);
					break;
				}

				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.pNext = nullptr;
				createInfo.flags = 0;
				createInfo.codeSize = size;
				createInfo.pCode = reinterpret_cast<const uint32_t*>(bin.pBegin);

				result = vkCreateShaderModule(device, &createInfo, nullptr, &fragment);

				pScratch->restore();
			}

			if (result != VK_SUCCESS) {
				vkDestroyShaderModule(device, vertex, nullptr);
				break;
			}

			VkPipelineShaderStageCreateInfo stageInfo[2]{};
			stageInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageInfo[0].pNext = nullptr;
			stageInfo[0].flags = 0;
			stageInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			stageInfo[0].module = vertex;
			stageInfo[0].pName = "main";
			stageInfo[0].pSpecializationInfo = nullptr;

			stageInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageInfo[1].pNext = nullptr;
			stageInfo[1].flags = 0;
			stageInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			stageInfo[1].module = fragment;
			stageInfo[1].pName = "main";
			stageInfo[1].pSpecializationInfo = nullptr;

			VkVertexInputBindingDescription bindings = { 0, sizeof(Vertex::_2D), VK_VERTEX_INPUT_RATE_VERTEX };
			std::array<VkVertexInputAttributeDescription, 3> attributes = { {
				{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex::_2D, position)},
				{1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex::_2D, uv)},
				{2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(Vertex::_2D, color)}
			} };
				
			VkPipelineVertexInputStateCreateInfo vertexInputState{};
			vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputState.pNext = nullptr;
			vertexInputState.flags = 0;
			vertexInputState.vertexBindingDescriptionCount = 0;
			vertexInputState.pVertexBindingDescriptions = nullptr;
			vertexInputState.vertexAttributeDescriptionCount = 0;
			vertexInputState.pVertexAttributeDescriptions = nullptr;

			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
			inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyState.pNext = nullptr;
			inputAssemblyState.flags = 0;
			inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyState.primitiveRestartEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tessellationState{};
			tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tessellationState.pNext = nullptr;
			tessellationState.flags = 0;
			tessellationState.patchControlPoints = 3;

			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.pNext = nullptr;
			viewportState.flags = 0;
			viewportState.viewportCount = 1;
			viewportState.pViewports = nullptr;
			viewportState.scissorCount = 1;
			viewportState.pScissors = nullptr;

			VkPipelineRasterizationStateCreateInfo rasterizationState{};
			rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationState.pNext = nullptr;
			rasterizationState.flags = 0;
			rasterizationState.depthClampEnable = VK_FALSE;
			rasterizationState.rasterizerDiscardEnable = VK_FALSE;
			rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationState.cullMode = VK_CULL_MODE_NONE;
			rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizationState.depthBiasEnable = VK_FALSE;
			rasterizationState.depthBiasConstantFactor = 0.0f;
			rasterizationState.depthBiasClamp = 0.0f;
			rasterizationState.depthBiasSlopeFactor = 0.0f;
			rasterizationState.lineWidth = 1.0f;

			VkPipelineMultisampleStateCreateInfo multisampleState{};
			multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleState.pNext = nullptr;
			multisampleState.flags = 0;
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampleState.sampleShadingEnable = VK_FALSE;
			multisampleState.minSampleShading = 1.0f;
			multisampleState.pSampleMask = nullptr;
			multisampleState.alphaToCoverageEnable = VK_FALSE;
			multisampleState.alphaToOneEnable = VK_FALSE;

			VkPipelineDepthStencilStateCreateInfo depthStencilState{};
			depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilState.pNext = nullptr;
			depthStencilState.flags = 0;
			depthStencilState.depthTestEnable = VK_FALSE;
			depthStencilState.depthWriteEnable = VK_FALSE;
			depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
			depthStencilState.depthBoundsTestEnable = VK_FALSE;
			depthStencilState.stencilTestEnable = VK_FALSE;
			depthStencilState.front = {};
			depthStencilState.back = {};
			depthStencilState.minDepthBounds = 0.0f;
			depthStencilState.maxDepthBounds = 1.0f;

			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

			VkPipelineColorBlendStateCreateInfo colorBlendState{};
			colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendState.pNext = nullptr;
			colorBlendState.flags = 0;
			colorBlendState.logicOpEnable = VK_FALSE;
			colorBlendState.logicOp = VK_LOGIC_OP_COPY;
			colorBlendState.attachmentCount = 1;
			colorBlendState.pAttachments = &colorBlendAttachment;
			colorBlendState.blendConstants[0] = 0.0f;
			colorBlendState.blendConstants[1] = 0.0f;
			colorBlendState.blendConstants[2] = 0.0f;
			colorBlendState.blendConstants[3] = 0.0f;

			std::array dynamicStates{
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.pNext = nullptr;
			dynamicState.flags = 0;
			dynamicState.dynamicStateCount = 2;
			dynamicState.pDynamicStates = dynamicStates.data();

			VkGraphicsPipelineCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.stageCount = 2;
			createInfo.pStages = stageInfo;
			createInfo.pVertexInputState = &vertexInputState;
			createInfo.pInputAssemblyState = &inputAssemblyState;
			createInfo.pTessellationState = nullptr;
			createInfo.pViewportState = &viewportState;
			createInfo.pRasterizationState = &rasterizationState;
			createInfo.pMultisampleState = &multisampleState;
			createInfo.pDepthStencilState = &depthStencilState;
			createInfo.pColorBlendState = &colorBlendState;
			createInfo.pDynamicState = &dynamicState;
			createInfo.layout = _pipelineLayout;
			createInfo.renderPass = _Canvas->renderPass;
			createInfo.subpass = 0;
			createInfo.basePipelineHandle = VK_NULL_HANDLE;
			createInfo.basePipelineIndex = 0;

			result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &_pipeline);

			vkDestroyShaderModule(device, fragment, nullptr);
			vkDestroyShaderModule(device, vertex, nullptr);
		}

		if (result != VK_SUCCESS)
			break;

		RenderBox const renderBox = new(std::nothrow) RenderBox_T;

		if (!renderBox)
			break;

		renderBox->pipeline = _pipeline;
		renderBox->pipelineLayout = _pipelineLayout;
		renderBox->descriptorSetLayout = _descriptorSetLayout;
		renderBox->sampler = _sampler;

		renderBox->device = device;

		*pRenderBox = renderBox;

		return 0;

	} while (false);

	if (_pipeline)
		vkDestroyPipeline(device, _pipeline, nullptr);

	if (_pipelineLayout)
		vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);

	if (_descriptorSetLayout)
		vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);

	if (_sampler)
		vkDestroySampler(device, _sampler, nullptr);

	return -1;
}

void destroyRenderBox(RenderBox const _RenderBox) noexcept {
	const VkDevice device = _RenderBox->device;

	vkDestroyPipeline(device, _RenderBox->pipeline, nullptr);
	vkDestroyPipelineLayout(device, _RenderBox->pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, _RenderBox->descriptorSetLayout, nullptr);
	vkDestroySampler(device, _RenderBox->sampler, nullptr);

	delete _RenderBox;
}

struct Renderer_T {
	VkDevice device;
	VkQueue graphics;
	VkQueue present;

	VkCommandPool commandPool;
	mem::span<VkCommandBuffer> commandBuffer;
	mem::span<VkSemaphore> imageSemaphore;
	mem::span<VkSemaphore> renderSemaphore;
	mem::span<VkFence> frameFence;
	
	uint32_t bufferedFrames;
	uint32_t frame;
};

int createRenderer(Emulator const _Emulator, Canvas const _Canvas, RenderBox const _RenderBox, const RendererCreateInfo* const pCreateInfo, Renderer* const pRenderer) noexcept {
	const VkDevice device = _Emulator->device;

	VkCommandPool _commandPool = VK_NULL_HANDLE;
	mem::span<VkCommandBuffer> _commandBuffer;
	mem::span<VkSemaphore> _imageSemaphore;
	mem::span<VkSemaphore> _renderSemaphore;
	mem::span<VkFence> _frameFence;

	do {
		VkResult result;
		
		{
			VkCommandPoolCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			createInfo.queueFamilyIndex = _Emulator->queueFamily.graphics;

			result = vkCreateCommandPool(device, &createInfo, nullptr, &_commandPool);
		}

		if (result != VK_SUCCESS)
			break;

		_commandBuffer = { new(std::nothrow) VkCommandBuffer[pCreateInfo->maxFrameBuffering], (size_t)pCreateInfo->maxFrameBuffering };

		if (!_commandBuffer)
			break;

		{
			VkCommandBufferAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocateInfo.pNext = nullptr;
			allocateInfo.commandPool = _commandPool;
			allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocateInfo.commandBufferCount = pCreateInfo->maxFrameBuffering;

			result = vkAllocateCommandBuffers(device, &allocateInfo, _commandBuffer);
		}

		if (result != VK_SUCCESS)
			break;

		_imageSemaphore = { new(std::nothrow) VkSemaphore[pCreateInfo->maxFrameBuffering], (size_t)pCreateInfo->maxFrameBuffering };

		if (!_imageSemaphore)
			break;

		_imageSemaphore.assign_default();

		{
			VkSemaphoreCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;

			const VkSemaphore* const pSemaphoreEnd = _imageSemaphore.pEnd;
			for (VkSemaphore* pSemaphore{ _imageSemaphore.pBegin }; pSemaphore != pSemaphoreEnd && result == VK_SUCCESS; ++pSemaphore)
				result = vkCreateSemaphore(device, &createInfo, nullptr, pSemaphore);
		}		

		if (result != VK_SUCCESS)
			break;

		const size_t swapchainImageCount = _Canvas->image.size();

		_renderSemaphore = { new(std::nothrow) VkSemaphore[swapchainImageCount], swapchainImageCount };

		if (!_renderSemaphore)
			break;

		_renderSemaphore.assign_default();

		{
			VkSemaphoreCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;

			const VkSemaphore* const pSemaphoreEnd = _renderSemaphore.pEnd;
			for (VkSemaphore* pSemaphore{ _renderSemaphore.pBegin }; pSemaphore != pSemaphoreEnd && result == VK_SUCCESS; ++pSemaphore)
				result = vkCreateSemaphore(device, &createInfo, nullptr, pSemaphore);
		}

		if (result != VK_SUCCESS)
			break;

		_frameFence = { new(std::nothrow) VkFence[pCreateInfo->maxFrameBuffering], (size_t)pCreateInfo->maxFrameBuffering };		

		if (!_frameFence)
			break;

		{
			VkFenceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			const VkFence* const pFenceEnd = _frameFence.pEnd;
			for (VkFence* pFence{ _frameFence.pBegin }; pFence != pFenceEnd && result == VK_SUCCESS; ++pFence)
				result = vkCreateFence(device, &createInfo, nullptr, pFence);
		}

		if (result != VK_SUCCESS)
			break;

		

		Renderer const renderer = new(std::nothrow) Renderer_T;

		if (!renderer)
			break;
		
		renderer->frame = 0;
		renderer->bufferedFrames = pCreateInfo->maxFrameBuffering;

		renderer->frameFence = _frameFence;
		renderer->renderSemaphore = _renderSemaphore;
		renderer->imageSemaphore = _imageSemaphore;
		renderer->commandBuffer = _commandBuffer;
		renderer->commandPool = _commandPool;

		renderer->present = _Emulator->queue.present;
		renderer->graphics = _Emulator->queue.graphics;
		renderer->device = device;

		*pRenderer = renderer;

		return 0;

	} while (false);

	if (_frameFence) {
		const VkFence* const pFenceEnd = _frameFence.pEnd;
		for (const VkFence* pFence{ _frameFence.pBegin }; pFence != pFenceEnd && *pFence; ++pFence)
			vkDestroyFence(device, *pFence, nullptr);

		delete[] _frameFence;
	}

	if (_renderSemaphore) {
		const VkSemaphore* const pSemaphoreEnd = _renderSemaphore.pEnd;
		for (const VkSemaphore* pSemaphore{ _renderSemaphore.pBegin }; pSemaphore != pSemaphoreEnd && *pSemaphore; ++pSemaphore)
			vkDestroySemaphore(device, *pSemaphore, nullptr);

		delete[] _renderSemaphore;
	}

	if (_imageSemaphore) {
		const VkSemaphore* const pSemaphoreEnd = _imageSemaphore.pEnd;
		for (const VkSemaphore* pSemaphore{ _imageSemaphore.pEnd }; pSemaphore != pSemaphoreEnd && *pSemaphore; ++pSemaphore)
			vkDestroySemaphore(device, *pSemaphore, nullptr);

		delete[] _imageSemaphore;
	}

	if (_commandBuffer)
		delete[] _commandBuffer;

	if (_commandPool)
		vkDestroyCommandPool(device, _commandPool, nullptr);

	return -1;
}

void destroyRenderer(Renderer const _Renderer) noexcept {
	const VkDevice device = _Renderer->device;

	const VkFence* const pFenceEnd = _Renderer->frameFence.pEnd;
	for (const VkFence* pFence{ _Renderer->frameFence.pBegin }; pFence != pFenceEnd; ++pFence)
		vkDestroyFence(device, *pFence, nullptr);

	delete[] _Renderer->frameFence;

	const VkSemaphore* const pRenderSemaphoreEnd = _Renderer->renderSemaphore.pEnd;
	for (const VkSemaphore* pRenderSemaphore{ _Renderer->renderSemaphore.pBegin }; pRenderSemaphore != pRenderSemaphoreEnd; ++pRenderSemaphore)
		vkDestroySemaphore(device, *pRenderSemaphore, nullptr);

	delete[] _Renderer->renderSemaphore;

	const VkSemaphore* const pImageSemaphoreEnd = _Renderer->imageSemaphore.pEnd;
	for (const VkSemaphore* pImageSemaphore{ _Renderer->imageSemaphore.pBegin }; pImageSemaphore != pImageSemaphoreEnd; ++pImageSemaphore)
		vkDestroySemaphore(device, *pImageSemaphore, nullptr);

	delete[] _Renderer->imageSemaphore;

	delete[] _Renderer->commandBuffer;

	vkDestroyCommandPool(device, _Renderer->commandPool, nullptr);

	delete _Renderer;
}

int waitRenderer(Renderer const _Renderer) noexcept {
	const VkDevice device = _Renderer->device;

	do {
		VkResult result;

		result = vkQueueWaitIdle(_Renderer->graphics);

		if (result == VK_TIMEOUT)
			return 1;

		if (result != VK_SUCCESS)
			break;

		result = vkQueueWaitIdle(_Renderer->present);

		if (result == VK_TIMEOUT)
			return 1;

		if (result != VK_SUCCESS)
			break;
		
		return 0;

	} while (false);

	return -1;
}

int draw(Canvas const _Canvas, Renderer const _Renderer, RenderBox const _RenderBox) noexcept {
	const VkDevice device = _Renderer->device;

	do {
		VkResult result;
		
		const uint32_t frame = _Renderer->frame;

		const VkSwapchainKHR swapchain = _Canvas->swapchain;
		const VkSemaphore imageAvailable = _Renderer->imageSemaphore[frame];

		const VkFence frameFence = _Renderer->frameFence[frame];
		
		result = vkWaitForFences(device, 1u, &frameFence, VK_TRUE, UINT64_MAX);

		if (result != VK_SUCCESS)
			break;
		
		uint32_t imageIndex;
		result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			return 1;

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			break;
		
		VkFence imageFence = _Canvas->fence[imageIndex];

		if (!imageFence)
			imageFence = frameFence;

		result = vkWaitForFences(device, 1u, &imageFence, VK_TRUE, UINT64_MAX);

		if (result != VK_SUCCESS)
			break;
		
		const VkCommandBuffer commandBuffer = _Renderer->commandBuffer[frame];

		result = vkResetCommandBuffer(commandBuffer, 0);

		if (result != VK_SUCCESS)
			break;

		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		}

		if (result != VK_SUCCESS)
			break;

		const VkRect2D renderArea = { {}, _Canvas->extent };

		{
			VkRenderPassBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.renderPass = _Canvas->renderPass;
			beginInfo.framebuffer = _Canvas->framebuffer[imageIndex];
			beginInfo.renderArea = renderArea;
			beginInfo.clearValueCount = static_cast<uint32_t>(_Canvas->clearValue.size());
			beginInfo.pClearValues = _Canvas->clearValue;

			vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _RenderBox->pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = renderArea.extent.width;
		viewport.height = renderArea.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = renderArea.extent;

		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		result = vkEndCommandBuffer(commandBuffer);

		if (result != VK_SUCCESS)
			break;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		const VkSemaphore renderSemaphore = _Renderer->renderSemaphore[frame];

		result = vkResetFences(device, 1u, &frameFence);

		if (result != VK_SUCCESS)
			break;

		{
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pNext = nullptr;
			submitInfo.waitSemaphoreCount = 1u;
			submitInfo.pWaitSemaphores = &imageAvailable;
			submitInfo.pWaitDstStageMask = &waitStage;
			submitInfo.commandBufferCount = 1u;
			submitInfo.pCommandBuffers = &commandBuffer;
			submitInfo.signalSemaphoreCount = 1u;
			submitInfo.pSignalSemaphores = &renderSemaphore;

			result = vkQueueSubmit(_Renderer->graphics, 1u, &submitInfo, _Renderer->frameFence[frame]);
		}

		if (result != VK_SUCCESS)
			break;

		_Canvas->fence[imageIndex] = frameFence;

		{
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.pNext = nullptr;
			presentInfo.waitSemaphoreCount = 1u;
			presentInfo.pWaitSemaphores = &renderSemaphore;
			presentInfo.swapchainCount = 1u;
			presentInfo.pSwapchains = &swapchain;
			presentInfo.pResults = nullptr;
			presentInfo.pImageIndices = &imageIndex;

			result = vkQueuePresentKHR(_Renderer->present, &presentInfo);
		}
		
		_Renderer->frame++;
		_Renderer->frame = _Renderer->frame == _Renderer->bufferedFrames ? 0 : _Renderer->frame;

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			return 1;

		if (result != VK_SUCCESS)
			break;

		return 0;

	} while (false);

	return -1;
}

struct ResourceSet_T {
	mem::static_vector<Image> image;
	mem::static_vector<VkImageView> imageview;
	mem::static_vector<Buffer> buffer;
	mem::static_vector<VkBufferView> bufferview;
};

struct Model {
	Buffer vertex;
	Buffer index;
	size_t vertCount;
};

int defferedLoadObject(AsyncLoader const _AsyncLoader, ResourceSet const _ResourceSet, const RenderObjectCreateInfo* const pCreateInfo) noexcept {
	Buffer _vertexBuffer{};
	Buffer _indexBuffer{};

	do {
		VkResult result;

		{
			VkBufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.size = static_cast<VkDeviceSize>(pCreateInfo->vertexBufferSize);
			createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;

			VmaAllocationCreateInfo allocationInfo{};
			allocationInfo.flags = 0;
			allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			allocationInfo.requiredFlags = 0;
			allocationInfo.preferredFlags = 0;
			allocationInfo.memoryTypeBits = 0;
			allocationInfo.pool = nullptr;
			allocationInfo.pUserData = nullptr;
			allocationInfo.priority = 0;

			result = vmaCreateBuffer(_AsyncLoader->allocator, &createInfo, &allocationInfo, &_vertexBuffer.buffer, &_vertexBuffer.allocation, nullptr);

			if (result != VK_SUCCESS)
				break;

			createInfo.size = static_cast<VkDeviceSize>(pCreateInfo->indexBufferSize);
			createInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

			result = vmaCreateBuffer(_AsyncLoader->allocator, &createInfo, &allocationInfo, &_indexBuffer.buffer, &_indexBuffer.allocation, nullptr);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			//result = vkBeginCommandBuffer();
		}

		rndr::DataSource source = { pCreateInfo->pVertex, pCreateInfo->vertexBufferSize };
		VkDeviceSize write = 0;

		VkBufferCopy copy;

		_AsyncLoader->stage.streamBufferUpload(&source, &write, &copy);

	} while (false);

	return -1;
}
