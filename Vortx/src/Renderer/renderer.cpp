#include "renderer.h"

#include <vulkan/vulkan_win32.h>
#include <array>

#include <assets/io/io.h>

constexpr uint32_t INVALID_QUEUE_FAMILY = UINT32_MAX;

int Renderer::deploy() noexcept {
	VkResult result;

	mem::stack _stash(256ull << 10);

	VkInstance _instance = VK_NULL_HANDLE;
	mem::span<PhysicalDevice> _physicalDevices;

	do {

		mem::scratch.mark();

		{
			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pNext = nullptr;
			appInfo.pApplicationName = "My Application";
			appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
			appInfo.pEngineName = "My Engine";
			appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_3;

#ifdef _DEBUG
			constexpr std::array<const char*, 1> instanceLayers{
				"VK_LAYER_KHRONOS_validation"
			};

			constexpr std::array<const char*, 3> instanceExtensions{
				"VK_KHR_surface",
				"VK_KHR_win32_surface",
				"VK_EXT_debug_utils"
			};
#else
			constexpr std::array<const char*, 0> instanceLayers{

			};

			constexpr std::array<const char*, 2> instanceExtensions{
				"VK_KHR_surface",
				"VK_KHR_win32_surface"
			};
#endif

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

		_physicalDevices = _stash.alloc<PhysicalDevice>(physicalDeviceCount);

		mem::span<VkPhysicalDevice> devicehandles = mem::scratch.alloc<VkPhysicalDevice>(physicalDeviceCount);
		
		result = vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, devicehandles);
		
		if (result != VK_SUCCESS)
			break;

		for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
			VkPhysicalDevice handle = devicehandles[i];

			vkGetPhysicalDeviceProperties(handle, &_physicalDevices[i].properties);
			vkGetPhysicalDeviceMemoryProperties(handle, &_physicalDevices[i].memory);
			vkGetPhysicalDeviceFeatures(handle, &_physicalDevices[i].features);

			_physicalDevices[i].handle = handle;
		}

		if (deployBase.marked()) {
			vkDestroyInstance(instance, nullptr);
		}

		stash = std::move(_stash);
		
		physicalDevices = _physicalDevices;

		instance = _instance;
		
		deployBase = stash.mark();

		mem::scratch.restore();

		return 0;

	} while (false);

	mem::scratch.restore();

	if(_instance)
		vkDestroyInstance(_instance, nullptr);

	return -1;
}

void Renderer::enumeratePhysicalDeviceNames(size_t* pCount, const char** const pDeviceName) noexcept {
	assert(pCount);
	
	if (pDeviceName) {
		const size_t writeCount = *pCount;

		for (size_t i{}; i < writeCount; ++i) {
			pDeviceName[i] = physicalDevices[i].properties.deviceName;
		}

		return;
	}

	*pCount = physicalDevices.size();
}

int Renderer::createDevice(HINSTANCE hinstance, HWND hwnd, size_t physicalDeviceIndex) {
	VkSurfaceKHR _surface = VK_NULL_HANDLE;

	VkDevice _device = VK_NULL_HANDLE;

	VkCommandPool _graphicsCommandPool = VK_NULL_HANDLE;
	VkCommandPool _transferCommandPool = VK_NULL_HANDLE;

	VmaAllocator _allocator = VK_NULL_HANDLE;

	VkBuffer _stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation _stagingAllocation = VK_NULL_HANDLE;
	mem::span<uint8_t> _stagingSpan;

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	mem::span<VkImageView> _swapchainImageView;

	RenderPasses _renderPass;

	Samplers _sampler;

	DescriptorSetlayouts _descriptorSetLayout;

	PipelineLayouts _pipelineLayout;
	Pipelines _pipeline;

	VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;

	VkSemaphore _imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore _transferCompleteSemaphore = VK_NULL_HANDLE;

	VkFence _fence = VK_NULL_HANDLE;

	do {

		VkResult result;

		mem::scratch.mark();

		{
			VkWin32SurfaceCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.hinstance = hinstance;
			createInfo.hwnd = hwnd;

			result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &_surface);
		}

		if (result != VK_SUCCESS)
			break;

		const VkPhysicalDevice _physicalDevice = physicalDevices[physicalDeviceIndex].handle;

		mem::static_vector<uint32_t> uniqueQueueFamilies = mem::scratch.alloc<uint32_t>(3);

		QueueFamilyIndices _familyIndex = { INVALID_QUEUE_FAMILY, INVALID_QUEUE_FAMILY, INVALID_QUEUE_FAMILY };

		{
			uint32_t queueFamilyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);

			mem::span<VkQueueFamilyProperties> queueFamilies = mem::scratch.alloc<VkQueueFamilyProperties>(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilies);

			mem::span<uint32_t> familyRole = mem::scratch.alloc<uint32_t>(queueFamilyCount);
			familyRole.assign_default();

			constexpr VkQueueFlags releventCapabilities = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

			for (uint32_t i = 0; i < queueFamilyCount; ++i) {
				VkQueueFlags queueFlags = queueFamilies[i].queueFlags & releventCapabilities;
				familyRole[i] = __popcnt(queueFlags);
			}

			uint32_t min = UINT32_MAX;

			for (uint32_t i = 0; i < queueFamilyCount; ++i) {
				if((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
					continue;

				if (familyRole[i] >= min)
					continue;

				min = familyRole[i];
				_familyIndex.graphics = i;
				uniqueQueueFamilies.push_back_unique(i);

			}

			min = UINT32_MAX;

			for (uint32_t i = 0; i < queueFamilyCount; ++i) {
				if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0)
					continue;

				if (familyRole[i] >= min)
					continue;

				min = familyRole[i];
				_familyIndex.transfer = i;
				uniqueQueueFamilies.push_back_unique(i);
			}

			VkBool32 presentSupport = VK_FALSE;

			for (uint32_t i = 0; i < queueFamilyCount; ++i) {
				vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &presentSupport);
				if (!presentSupport)
					continue;
				
				_familyIndex.present = i;
				uniqueQueueFamilies.push_back_unique(i);

				break;
			}
		}

		if (uniqueQueueFamilies.empty())
			break;
		

		{
			const size_t queueInfoCount = uniqueQueueFamilies.size();

			mem::span<VkDeviceQueueCreateInfo> queueInfos = mem::scratch.alloc<VkDeviceQueueCreateInfo>(queueInfoCount);

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

			if (physicalDevices[physicalDeviceIndex].features.samplerAnisotropy)
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

			result = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device);
		}

		if (result != VK_SUCCESS)
			break;

		DeviceQueues _deviceQueue{};

		vkGetDeviceQueue(_device, _familyIndex.graphics, 0, &_deviceQueue.graphics);
		vkGetDeviceQueue(_device, _familyIndex.transfer, 0, &_deviceQueue.transfer);
		vkGetDeviceQueue(_device, _familyIndex.present, 0, &_deviceQueue.present);

		{
			VkCommandPoolCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			createInfo.queueFamilyIndex = _familyIndex.graphics;

			result = vkCreateCommandPool(_device, &createInfo, nullptr, &_graphicsCommandPool);

			if (result != VK_SUCCESS)
				break;

			createInfo.queueFamilyIndex = _familyIndex.transfer;
			
			result = vkCreateCommandPool(_device, &createInfo, nullptr, &_transferCommandPool);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VmaAllocatorCreateInfo createInfo{};
			createInfo.flags = 0;
			createInfo.physicalDevice = _physicalDevice;
			createInfo.device = _device;
			createInfo.preferredLargeHeapBlockSize = 0;
			createInfo.pAllocationCallbacks = nullptr;
			createInfo.pDeviceMemoryCallbacks = nullptr;
			createInfo.pHeapSizeLimit = nullptr;
			createInfo.pVulkanFunctions = nullptr;
			createInfo.instance = instance;
			createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
			createInfo.pTypeExternalMemoryHandleTypes = nullptr;

			result = vmaCreateAllocator(&createInfo, &_allocator);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkBufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.size = VkDeviceSize(64) << 20;
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

			result = vmaCreateBuffer(_allocator, &createInfo, &allocationCreateInfo, &_stagingBuffer, &_stagingAllocation, &allocationInfo);

			_stagingSpan = mem::span<uint8_t>{ reinterpret_cast<uint8_t*>(allocationInfo.pMappedData), static_cast<size_t>(allocationInfo.size) };
		}

		if (result != VK_SUCCESS)
			break;
		
		VkSurfaceFormatKHR _surfaceFormat;

		{
			constexpr std::array preferredSurfaceFormats{
				VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
				VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
				VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
				VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
			};

			uint32_t surfaceFormatCount;

			result = vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &surfaceFormatCount, nullptr);

			if (result != VK_SUCCESS)
				break;

			mem::span<VkSurfaceFormatKHR> surfaceFormats = mem::scratch.alloc<VkSurfaceFormatKHR>(surfaceFormatCount);

			result = vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &surfaceFormatCount, surfaceFormats);
			
			if (result != VK_SUCCESS)
				break;

			const VkSurfaceFormatKHR* pSurfaceFormat = nullptr;

			const VkSurfaceFormatKHR* const pEnd = preferredSurfaceFormats.data() + preferredSurfaceFormats.size();
			for (const VkSurfaceFormatKHR* pFormat{ preferredSurfaceFormats.data() }; pFormat != pEnd; ++pFormat) {
				pSurfaceFormat = std::find_if(surfaceFormats.pBegin, surfaceFormats.pEnd, [pFormat](const VkSurfaceFormatKHR& _Format) { return _Format.format == pFormat->format && _Format.colorSpace == pFormat->colorSpace; });

				if (pSurfaceFormat != surfaceFormats.pEnd)
					break;

				pSurfaceFormat = nullptr;
			}

			if (!pSurfaceFormat)
				break;

			_surfaceFormat = *pSurfaceFormat;
		}

		VkPresentModeKHR _presentMode;

		{
			uint32_t presentModeCount;

			result = vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, nullptr);

			if (result != VK_SUCCESS)
				break;

			mem::span<VkPresentModeKHR> presentModes = mem::scratch.alloc<VkPresentModeKHR>(presentModeCount);

			result = vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, presentModes);

			if (result != VK_SUCCESS)
				break;

			VkPresentModeKHR* pPresentMode = std::find(presentModes.pBegin, presentModes.pEnd, VK_PRESENT_MODE_MAILBOX_KHR);

			_presentMode = pPresentMode != presentModes.pEnd ? *pPresentMode : VK_PRESENT_MODE_FIFO_KHR;
		}

		VkSurfaceCapabilitiesKHR capabilities;

		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &capabilities);

		if (result != VK_SUCCESS)
			break;

		uint32_t minImageCount;

		{
			constexpr uint32_t preferredImageCount = 2u;

			minImageCount = preferredImageCount > capabilities.minImageCount ? preferredImageCount : capabilities.minImageCount;

			if (capabilities.maxImageCount) {
				minImageCount = minImageCount < capabilities.maxImageCount ? minImageCount : capabilities.maxImageCount;
			}
		}

		{
			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.surface = _surface;
			createInfo.minImageCount = minImageCount;
			createInfo.imageFormat = _surfaceFormat.format;
			createInfo.imageColorSpace = _surfaceFormat.colorSpace;
			createInfo.imageExtent = capabilities.currentExtent;
			createInfo.imageArrayLayers = 1u;
			createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
			createInfo.preTransform = capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = _presentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			result = vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain);
		}

		if (result != VK_SUCCESS)
			break;

		mem::span<VkImage> _swapchainImage;

		{
			uint32_t swapchainImageCount;
		
			result = vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, nullptr);

			if (result != VK_SUCCESS)
				break;

			_swapchainImage = mem::scratch.alloc<VkImage>(swapchainImageCount);

			result = vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, _swapchainImage);
			
			if (result != VK_SUCCESS)
				break;

			_swapchainImageView = mem::scratch.alloc<VkImageView>(swapchainImageCount);

			_swapchainImageView.assign_default();

			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = _surfaceFormat.format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			for (uint32_t i = 0; i < swapchainImageCount; ++i) {
				createInfo.image = _swapchainImage[i];

				result = vkCreateImageView(_device, &createInfo, nullptr, &_swapchainImageView[i]);

				if (result != VK_SUCCESS)
					break;
			}
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkAttachmentDescription attachment{};
			attachment.format = _surfaceFormat.format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
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

			result = vkCreateRenderPass(_device, &createInfo, nullptr, &_renderPass.composite);
		}

		if (result != VK_SUCCESS)
			break;

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

			result = vkCreateSampler(_device, &createInfo, nullptr, &_sampler.raw);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = 0;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			binding.pImmutableSamplers = &_sampler.raw;

			VkDescriptorSetLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.bindingCount = 1;
			createInfo.pBindings = &binding;

			result = vkCreateDescriptorSetLayout(_device, &createInfo, nullptr, &_descriptorSetLayout.imageRead);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkPipelineLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.setLayoutCount = 1;
			createInfo.pSetLayouts = &_descriptorSetLayout.imageRead;
			createInfo.pushConstantRangeCount = 0;
			createInfo.pPushConstantRanges = nullptr;

			result = vkCreatePipelineLayout(_device, &createInfo, nullptr, &_pipelineLayout.composite);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkShaderModule vertex;

			{
				size_t size;

				int error = io::loadBinary(L"shaders/presentation.vert.spv", &size, nullptr);

				if (error)
					break;

				mem::scratch.mark();

				mem::span<uint8_t> bin = mem::scratch.alloc<uint8_t>(size);

				error = io::loadBinary(L"shaders/presentation.vert.spv", &size, bin);

				if (error)
					break;

				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.pNext = nullptr;
				createInfo.flags = 0;
				createInfo.codeSize = size;
				createInfo.pCode = reinterpret_cast<const uint32_t*>(bin.pBegin);

				result = vkCreateShaderModule(_device, &createInfo, nullptr, &vertex);

				mem::scratch.restore();
			}

			if (result != VK_SUCCESS)
				break;

			VkShaderModule fragment;

			{
				size_t size;

				int error = io::loadBinary(L"shaders/presentation.frag.spv", &size, nullptr);

				if (error) {
					vkDestroyShaderModule(_device, vertex, nullptr);
					break;
				}

				mem::scratch.mark();

				mem::span<uint8_t> bin = mem::scratch.alloc<uint8_t>(size);

				error = io::loadBinary(L"shaders/presentation.frag.spv", &size, bin);

				if (error) {
					vkDestroyShaderModule(_device, vertex, nullptr);
					break;
				}

				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.pNext = nullptr;
				createInfo.flags = 0;
				createInfo.codeSize = size;
				createInfo.pCode = reinterpret_cast<const uint32_t*>(bin.pBegin);

				result = vkCreateShaderModule(_device, &createInfo, nullptr, &fragment);

				mem::scratch.restore();
			}

			if (result != VK_SUCCESS) {
				vkDestroyShaderModule(_device, vertex, nullptr);
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
			createInfo.layout = _pipelineLayout.composite;
			createInfo.renderPass = _renderPass.composite;
			createInfo.subpass = 0;
			createInfo.basePipelineHandle = VK_NULL_HANDLE;
			createInfo.basePipelineIndex = 0;

			result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &_pipeline.composite);

			vkDestroyShaderModule(_device, fragment, nullptr);
			vkDestroyShaderModule(_device, vertex, nullptr);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSize.descriptorCount = 1;

			VkDescriptorPoolCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.maxSets = 1;
			createInfo.poolSizeCount = 1;
			createInfo.pPoolSizes = &poolSize;

			result = vkCreateDescriptorPool(_device, &createInfo, nullptr, &_descriptorPool);
		}

		if (result != VK_SUCCESS)
			break;

		VkDescriptorSet _descriptorSet;

		{
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.pNext = nullptr;
			allocateInfo.descriptorPool = _descriptorPool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &_descriptorSetLayout.imageRead;

			result = vkAllocateDescriptorSets(_device, &allocateInfo, &_descriptorSet);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkSemaphoreCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;

			result = vkCreateSemaphore(_device, &createInfo, nullptr, &_imageAvailableSemaphore);

			if (result != VK_SUCCESS)
				break;

			result = vkCreateSemaphore(_device, &createInfo, nullptr, &_transferCompleteSemaphore);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkFenceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			result = vkCreateFence(_device, &createInfo, nullptr, &_fence);
		}

		if (result != VK_SUCCESS)
			break;

		if (deviceBase.marked()) {
			vkDestroyFence(device, fence, nullptr);

			vkDestroySemaphore(device, transferCompleteSemaphore, nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
			
			vkDestroyDescriptorPool(device, descriptorPool, nullptr);

			vkDestroyPipeline(device, pipeline.composite, nullptr);
			vkDestroyPipelineLayout(device, pipelineLayout.composite, nullptr);
			
			vkDestroyDescriptorSetLayout(device, descriptorSetLayout.imageRead, nullptr);
			vkDestroySampler(device, sampler.raw, nullptr);
			
			vkDestroyRenderPass(device, renderPass.composite, nullptr);
			
			for (const VkImageView* pImageView{ swapchainImageView.pBegin }; pImageView != swapchainImageView.pEnd; ++pImageView) {
				vkDestroyImageView(device, *pImageView, nullptr);
			}
			
			vkDestroySwapchainKHR(device, swapchain, nullptr);
			
			vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
			
			vmaDestroyAllocator(allocator);
			
			vkDestroyCommandPool(device, transferCommandPool, nullptr);
			vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
			
			vkDestroyDevice(device, nullptr);
			
			vkDestroySurfaceKHR(instance, surface, nullptr);
		}

		stash.restore(deployBase);

		surface = _surface;

		device = _device;

		familyIndex = _familyIndex;
		deviceQueue = _deviceQueue;

		graphicsCommandPool = _graphicsCommandPool;
		transferCommandPool = _transferCommandPool;

		allocator = _allocator;

		stagingBuffer = _stagingBuffer;
		stagingAllocation = _stagingAllocation;
		transferStage = rndr::TransferStage{ _stagingSpan };

		swapchain = _swapchain;
		surfaceFormat = _surfaceFormat;
		presentMode = _presentMode;
		swapchainImage = stash.alloc<VkImage>(_swapchainImage.size());
		swapchainImage.copy(_swapchainImage);
		swapchainImageView = stash.alloc<VkImageView>(_swapchainImageView.size());
		swapchainImageView.copy(_swapchainImageView);

		renderPass = _renderPass;

		sampler = _sampler;
		descriptorSetLayout = _descriptorSetLayout;

		pipelineLayout = _pipelineLayout;
		pipeline = _pipeline;

		descriptorPool = _descriptorPool;
		descriptorSet = _descriptorSet;

		imageAvailableSemaphore = _imageAvailableSemaphore;
		transferCompleteSemaphore = _transferCompleteSemaphore;

		fence = _fence;

		deviceBase = stash.mark();

		mem::scratch.restore();

		return 0;

	} while (false);

	if (_fence)
		vkDestroyFence(_device, _fence, nullptr);

	if (_transferCompleteSemaphore)
		vkDestroySemaphore(_device, _transferCompleteSemaphore, nullptr);

	if (_imageAvailableSemaphore)
		vkDestroySemaphore(_device, _imageAvailableSemaphore, nullptr);

	if (_descriptorPool)
		vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

	if (_pipeline.composite)
		vkDestroyPipeline(_device, _pipeline.composite, nullptr);

	if (_pipelineLayout.composite)
		vkDestroyPipelineLayout(_device, _pipelineLayout.composite, nullptr);

	if (_descriptorSetLayout.imageRead)
		vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout.imageRead, nullptr);

	if (_sampler.raw)
		vkDestroySampler(_device, _sampler.raw, nullptr);

	if (_renderPass.composite)
		vkDestroyRenderPass(_device, _renderPass.composite, nullptr);

	for (const VkImageView* pImageView{ _swapchainImageView.pBegin }; pImageView != _swapchainImageView.pEnd; ++pImageView) {
		vkDestroyImageView(_device, *pImageView, nullptr);
	}

	if (_swapchain)
		vkDestroySwapchainKHR(_device, _swapchain, nullptr);

	if (_stagingBuffer)
		vmaDestroyBuffer(_allocator, _stagingBuffer, _stagingAllocation);

	if (_allocator)
		vmaDestroyAllocator(_allocator);

	if (_transferCommandPool)
		vkDestroyCommandPool(_device, _transferCommandPool, nullptr);

	if (_graphicsCommandPool)
		vkDestroyCommandPool(_device, _graphicsCommandPool, nullptr);

	if (_device)
		vkDestroyDevice(_device, nullptr);

	if (_surface)
		vkDestroySurfaceKHR(instance, _surface, nullptr);

	return -1;
}

int Renderer::sustainImage(const void* const pData, size_t _Size, uint32_t _Width, uint32_t _Height) noexcept {
	VkResult result;

	uint32_t imageIndex;

	result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result != VK_SUCCESS)
		return -1;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

	do {

		{
			VkCommandBufferAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocateInfo.pNext = nullptr;
			allocateInfo.commandPool = transferCommandPool;
			allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocateInfo.commandBufferCount = 1;

			result = vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			beginInfo.pInheritanceInfo = nullptr;

			result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		}
		
		if (result != VK_SUCCESS)
			break;

		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = VK_ACCESS_NONE;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapchainImage[imageIndex];
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_NONE, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, 0, 1, &barrier);
		}

		{
			rndr::DataSource source = { pData, _Size };
			rndr::TransferImageAttributes imageAttr = { VkExtent3D{_Width, _Height, 1}, 4 };

			VkOffset3D offset{};

			VkBufferImageCopy region{};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			int allocResult = transferStage.streamImageUpload(&source, &imageAttr, &offset, &region);

			if (allocResult == -1) {
				break;
			}

			rndr::flushRanges flushRange;
			transferStage.flushStream(&flushRange);

			for (size_t i = 0; i < flushRange.count; ++i) {
				vmaFlushAllocation(allocator, stagingAllocation, flushRange.offset[i], flushRange.size[i]);
			}

			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, swapchainImage[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		}
			
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_NONE;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapchainImage[imageIndex];
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_NONE, 0, 0, nullptr, 0, 0, 1, &barrier);
		}

		result = vkEndCommandBuffer(commandBuffer);

		if (result != VK_SUCCESS)
			break;

		{
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			submitInfo.pNext = nullptr;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &transferCompleteSemaphore;

			result = vkQueueSubmit(deviceQueue.transfer, 1, &submitInfo, fence);
		}

		if (result != VK_SUCCESS)
			break;

		{
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.pNext = nullptr;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &transferCompleteSemaphore;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &swapchain;
			presentInfo.pImageIndices = &imageIndex;
			presentInfo.pResults = nullptr;

			result = vkQueuePresentKHR(deviceQueue.present, &presentInfo);
		}

		result = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

		if (result != VK_SUCCESS)
			break;

		vkFreeCommandBuffers(device, transferCommandPool, 1, &commandBuffer);

		//transferStage.restore();

		return 1;

	} while (false);

	if (commandBuffer)
		vkFreeCommandBuffers(device, transferCommandPool, 1, &commandBuffer);

	transferStage.restore();

	return -1;
}

int Renderer::createImage(const void* const pData, size_t _Size, uint32_t _Width, uint32_t _Height) noexcept {
	VkResult result;

	VkImage _image = VK_NULL_HANDLE;
	VmaAllocation _allocation = VK_NULL_HANDLE;

	do {
		{
			VkImageCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.imageType = VK_IMAGE_TYPE_2D;
			createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
			createInfo.extent = { _Width, _Height, 1 };
			createInfo.mipLevels = 1;
			createInfo.arrayLayers = 1;
			createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
			createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VmaAllocationCreateInfo allocationInfo{};
			allocationInfo.flags = 0;
			allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			allocationInfo.requiredFlags = 0;
			allocationInfo.preferredFlags = 0;
			allocationInfo.memoryTypeBits = 0;
			allocationInfo.pool = nullptr;
			allocationInfo.pUserData = nullptr;
			allocationInfo.priority = 0;

			result = vmaCreateImage(allocator, &createInfo, &allocationInfo, &_image, &_allocation, nullptr);
		}

		if (result != VK_SUCCESS)
			break;

	} while (false);

	if(_allocation)
		vmaDestroyImage(allocator, _image, _allocation);
	
	return -1;
}

void Renderer::reset() noexcept {
	if (deviceBase.marked()) {
		vkDestroyFence(device, fence, nullptr);

		vkDestroySemaphore(device, transferCompleteSemaphore, nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		vkDestroyPipeline(device, pipeline.composite, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout.composite, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout.imageRead, nullptr);
		vkDestroySampler(device, sampler.raw, nullptr);

		vkDestroyRenderPass(device, renderPass.composite, nullptr);

		for (const VkImageView* pImageView{ swapchainImageView.pBegin }; pImageView != swapchainImageView.pEnd; ++pImageView) {
			vkDestroyImageView(device, *pImageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapchain, nullptr);

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

		vmaDestroyAllocator(allocator);

		vkDestroyCommandPool(device, transferCommandPool, nullptr);
		vkDestroyCommandPool(device, graphicsCommandPool, nullptr);

		vkDestroyDevice(device, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
}

Renderer::~Renderer() noexcept {
	if (instance)
		vkDestroyInstance(instance, nullptr);
}
