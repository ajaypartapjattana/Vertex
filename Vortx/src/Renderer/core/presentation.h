#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace rndr {

	struct PresentationStageCreateInfo {
		uint32_t graphicsFamilyIndex;
		VkQueue graphicsQueue;
		uint32_t presentFamilyIndex;
		VkQueue presentQueue;
	};

	class PresentationStage {
	private:
		VkDevice r_device = VK_NULL_HANDLE;
		VkPhysicalDevice r_physicalDevice = VK_NULL_HANDLE;

		VkSurfaceFormatKHR surfaceFormat;

		VkRenderPass compositionPass = VK_NULL_HANDLE;

		VkSampler sampler = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		
		VkCommandPool commandPool = VK_NULL_HANDLE;
		
		std::vector<VkCommandBuffer> commandBuffers;

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> descriptorSets;

		struct {
			uint32_t graphicsFamily = 0;
			VkQueue graphicsQueue = VK_NULL_HANDLE;
			uint32_t presentFamily = 0;
			VkQueue presentQueue = VK_NULL_HANDLE;

		} execution;

		struct PresentationTarget {
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			VkPresentModeKHR presentMode{};

			VkSwapchainKHR swapchain = VK_NULL_HANDLE;
			std::vector<VkImageView> imageViews;
			std::vector<VkFramebuffer> framebuffers;
			std::vector<VkFence> imageFence;

			VkExtent2D extent{};
		};

		std::vector<PresentationTarget> presentationTargets;

		struct PresentStageJob {
			VkFence inFlight;

			VkSemaphore imageAvailable;
			VkSemaphore renderComplete;

			VkDescriptorSet descriptorSet;
			
			VkCommandBuffer commandBuffer;
		};

		std::vector<PresentStageJob> jobs;
		size_t acquireHint = 0;

		uint32_t maxConcurrentPresentations = 3;

	public:
		PresentationStage() = default;
		PresentationStage(const PresentationStage&) = delete;
		PresentationStage(PresentationStage&& other) = delete;
		
		PresentationStage& operator=(const PresentationStage&) = delete;
		PresentationStage& operator=(PresentationStage&& other) = delete;

		~PresentationStage() noexcept = default;

		VkResult root(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR primarySurface, const PresentationStageCreateInfo* const pQueueInfo) noexcept;

		VkResult targetSurface(VkSurfaceKHR _Surface, size_t* const pIndex) noexcept;
		void releaseSurface(size_t index, VkSurfaceKHR* pSurface) noexcept;

		VkResult configurePresentation(size_t targetIndex, uint32_t minImageCount) noexcept;
		
		VkResult presentationImagePool(const VkImageView* pImageViews, uint32_t count);
		VkResult stagePresentation(uint32_t imageIndex, uint32_t targetIndex, VkSemaphore waitSemaphore);

		void reset() noexcept;

	};

}