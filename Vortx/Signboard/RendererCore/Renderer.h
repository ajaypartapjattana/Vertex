#pragma once

#include "Signboard/RHI/vulkan/VulkanCommandBuffer.h"
#include "Signboard/RHI/vulkan/VulkanImage.h"

#include "Signboard/resources/ResourceAPI.h"

#include <array>
#include <vector>
#include <memory>

struct RenderQueue {
	std::vector<ObjectHandle> drawList;
};

class VulkanDevice;

class VulkanSwapchain;
class VulkanFrameBuffer;
class VulkanRenderPass;

class RenderContext;
class Scene;

class Renderer {
public:
	explicit Renderer(VulkanDevice& device, ResourceView view);
	~Renderer();

	//rendering logic ---
	void renderFrame();
	void resize();

private:
	// --- EXTERNS ---

	VulkanDevice& device;
	RenderContext& renderContext;
	Scene& scene;

	// --- FRAMESYNC ---
	static constexpr uint32_t FRAMES_IN_FLIGHT = 2;
	uint32_t frameIndex = 0;

	struct Frame {
		uint32_t frameIndex;
		VulkanCommandBuffer cmd;
		VulkanImage swapchainImage;
		ImageExtent2D extent;
	};

	std::array<Frame, FRAMES_IN_FLIGHT> frames;

	
	VkCommandPool commandPool;
	VkQueue queue;

private:
	//FRAMEDRAW OBJECTS ---

	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t currentFrame = 0;
	bool framebufferResized = false;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;
	VulkanImage MSAAImage;
	VulkanImage depthImage;

	std::vector<VkFence> imagesInFlight;
	uint32_t accquiredImageIndex;

private:
	//UTILITY ---

	bool constructDrawList(); //
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex); //

	void createDepthResources();
	void createMSAAResources();
	void createFramebuffers();

	void createRenderPass();

	void createFrameContext();
};
