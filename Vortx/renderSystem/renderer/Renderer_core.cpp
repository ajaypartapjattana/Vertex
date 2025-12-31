#include "Renderer.h"

#include <algorithm>

#include "renderer/context/helpers/ContextHelpers.h"
#include "renderer/utility/VulkanUtils.h"

Renderer::Renderer(const ContextHandle& ctx, GLFWwindow* window)
	: activeContext(ctx),
	  window(window),
	  
	  device(ctx.device),
	  physicalDevice(ctx.physicalDevice),
	  commandPool(ctx.commandPool),
	  queue(ctx.graphicsQueue),
	  descriptorPool(createDescriptorPool(MAX_FRAMES_IN_FLIGHT)),

	  objectSystem(device, physicalDevice, descriptorPool),
	  meshSystem(device, physicalDevice, commandPool, queue),
	  textureSystem(device, physicalDevice, commandPool, queue),
	  materialSystem(device, descriptorPool, textureSystem),
	  pipelineManager(device),
	  
	  systemView{ 
		  objectSystem,
		  meshSystem,
		  materialSystem,
		  pipelineManager 
	  }

{
	createSwapchain();
	createSwapchainImageViews();
	createRenderPass();
	createDepthResources();
	createMSAAResources();
	createFramebuffers();

	createFrameContext();

	forwardPass.init(device, systemView, renderPass);

	allocateGlobalDescriptorSets(MAX_FRAMES_IN_FLIGHT);
	writeGlobalDescriptorSets(MAX_FRAMES_IN_FLIGHT);
}

Renderer::~Renderer() {
	vkDeviceWaitIdle(device);

	std::vector<VkCommandBuffer> buffers;
	buffers.reserve(frames.size());
	for (auto& frame : frames) {
		vkDestroySemaphore(device, frame.imageAvailable_Semaphore, nullptr);
		vkDestroySemaphore(device, frame.renderFinished_Semaphore, nullptr);
		vkDestroyFence(device, frame.inFlight_Fence, nullptr);

		buffers.push_back(frame.commandBuffer);
	}
	cleanupSwapChain();

	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(buffers.size()), buffers.data());
	vkDestroyRenderPass(device, renderPass, nullptr);
}

void Renderer::registerPass(ForwardPass& pass) {
	PassID passID = passRegistry.registerPass(activePass);
	descriptorSetLayouts& layouts = pass.descriptorLayouts;

	globalSetLayouts[passID] = layouts.globalLayout;
	objectSystem.registerPass(passID, layouts.objectLayout);
	materialSystem.registerPass(passID, layouts.materialLayout);
}

void Renderer::createFrameContext() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

	std::vector<VkCommandBuffer> buffers(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateCommandBuffers(device, &allocInfo, buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		frames[i].commandBuffer = buffers[i];

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frames[i].imageAvailable_Semaphore) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &frames[i].inFlight_Fence) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronizers!");
		}
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frames[i].renderFinished_Semaphore) != VK_SUCCESS) {
			throw std::runtime_error("failed to create per-image renderFinished semaphore!");
		}
	}
}

void Renderer::createSwapchain() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, activeContext.surface);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = activeContext.surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indicies = ContextUtil::findQueueFamilies(physicalDevice, activeContext.surface);
	uint32_t queueFamilyIndices[] = { indicies.graphicsFamily.value(), indicies.presentFamily.value() };
	if (indicies.graphicsFamily != indicies.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
}
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableformat : availableFormats) {
		if (availableformat.format == VK_FORMAT_B8G8R8A8_SRGB && availableformat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableformat;
		}
	}
	return availableFormats[0];
}
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}
void Renderer::createSwapchainImageViews() {
	swapChainImageViews.resize(swapChainImages.size());
	for (int i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = VulkanUtils::createImageView(device, swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void Renderer::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat(physicalDevice);
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 2;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 1;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, colorAttachmentResolve, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;


	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void Renderer::createDepthResources() {
	depthImage.format = findDepthFormat(physicalDevice);
	depthImage.samples = msaaSamples;
	depthImage.aspectFlagBits = VK_IMAGE_ASPECT_DEPTH_BIT;
	VulkanUtils::createImageResources(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage);

	VulkanUtils::transitionImageLayout(commandPool, device, activeContext.graphicsQueue, depthImage.image, depthImage.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
void Renderer::createMSAAResources() {
	MSAAImage.format = swapChainImageFormat;
	MSAAImage.samples = msaaSamples;
	MSAAImage.aspectFlagBits = VK_IMAGE_ASPECT_COLOR_BIT;
	VulkanUtils::createImageResources(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MSAAImage);
}

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
	return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, physicalDevice);
}
VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physicalDevice) {
	for (VkFormat format : candidates) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

void Renderer::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = { MSAAImage.view, swapChainImageViews[i], depthImage.view };
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void Renderer::cleanupSwapChain() {
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

void Renderer::recreateSwapChain() {
	int widht, height = 0;
	glfwGetFramebufferSize(window, &widht, &height);
	while (widht == 0 || height == 0) {
		glfwGetFramebufferSize(window, &widht, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(device);

	cleanupSwapChain();
	imagesInFlight.clear();
	createSwapchain();

	createSwapchainImageViews();
	createDepthResources();
	createMSAAResources();
	createFramebuffers();
}

VkDescriptorPool Renderer::createDescriptorPool(uint16_t frameCount) {
	std::array<VkDescriptorPoolSize, 2> poolSizes;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(frameCount) + 500;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1000;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.maxSets = static_cast<uint32_t>(frameCount) + 500 + 500;
	poolInfo.pPoolSizes = poolSizes.data();

	VkDescriptorPool descPool;
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	return descriptorPool;
}

void Renderer::allocateGlobalDescriptorSets(uint32_t frameCount) {
	std::vector<VkDescriptorSetLayout> globalLayouts;

	for (uint32_t i = 0; i < frameCount; i++) {
		for (auto& [passId, globalSetLayout] : globalSetLayouts) {
			globalLayouts.push_back(globalSetLayout);
		}
	}
	std::vector<VkDescriptorSet> globalSets(globalLayouts.size());
	
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(globalLayouts.size());
	allocInfo.pSetLayouts = globalLayouts.data();

	if (vkAllocateDescriptorSets(device, &allocInfo, globalSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate global descriptor sets");
	}

	for (uint32_t i = 0; i < frameCount; i++) {
		VulkanUtils::createBuffer(physicalDevice, device, sizeof(Global_UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, frames[i].globalUBO, frames[i].globalUBOMemory);
		vkMapMemory(device, frames[i].globalUBOMemory, 0, sizeof(Global_UBO), 0, &frames[i].mappedPtr);
	}

	uint32_t layoutIndex = 0;
	for (uint32_t i = 0; i < frameCount; i++) {
		for (auto& [passId, globalSetLayout] : globalSetLayouts) {
			frames[i].globalDescriptors.at(passId) = globalSets[layoutIndex++];
		}
	}
}

void Renderer::writeGlobalDescriptorSets(uint32_t frameCount) {
	for (uint32_t i = 0; i < frameCount; i++) {
		for (auto& [passId, globalSetLayout] : globalSetLayouts) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = frames[i].globalUBO;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(Global_UBO);

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = frames[i].globalDescriptors.at(passId);
			write.dstBinding = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
		}
	}
}