#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

#include "RHI/Vulkan.h"

#include "core/resource.h"

#include "RenderPass/PassRegistry.h"
#include "RenderPass/ForwardPass.h"


struct FrameContext {
	VkCommandBuffer commandBuffer;

	VkSemaphore imageAvailable_Semaphore;
	VkSemaphore renderFinished_Semaphore;
	VkFence inFlight_Fence;

	std::unordered_map<PassID, VkDescriptorSet> globalDescriptors;
	VkBuffer globalUBO;
	VkDeviceMemory globalUBOMemory;
	void* mappedPtr;
};

struct ModelDesc {
	const void* vertices;
	size_t vertexSize;
	size_t vertexCount;

	const uint32_t* indices;
	uint32_t indexCount;

	std::string materialName;
	std::string albedoPath;
	std::string normalPath;
	MaterialParams materialParams;

	glm::mat4 transform = glm::mat4(1.0f);
};

struct CameraData {
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 position;
};

using ModelID = uint32_t;
static constexpr ModelID INVALID_MODEL = UINT32_MAX;

class Renderer {
public:
	explicit Renderer(VulkanContext& context, GLFWwindow* window);
	~Renderer();

	//resource handling ---
	ModelID createModel(ModelDesc& desc);

	void registerPass(ForwardPass& pass);

	//rendering logic ---
	bool beginFrame();
	void drawFrame(const CameraData& global);
	void endFrame();

	void updateGlobal_UBO(const CameraData& camera);

private:
	//EXTERNS ---

	VulkanContext& activeContext;
	VulkanDevice& device

	VkCommandPool commandPool;
	VkQueue queue;

	GLFWwindow* window;

private:
	//RESP ---

	VkDescriptorPool descriptorPool;
	std::unordered_map<PassID, VkDescriptorSetLayout> globalSetLayouts;

	ObjectSystem objectSystem;
	MaterialSystem materialSystem;
	MeshSystem meshSystem;
	TextureSystem textureSystem;
	PipelineManager pipelineManager;

	RenderSystemView systemView;

private:
	//FRAMEDRAW OBJECTS ---

	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t currentFrame = 0;
	bool framebufferResized = false;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_4_BIT;

	VkRenderPass renderPass;
	ImageResources MSAAImage;
	ImageResources depthImage;

	std::vector<VkFence> imagesInFlight;
	std::array<FrameContext, MAX_FRAMES_IN_FLIGHT> frames;
	uint32_t accquiredImageIndex;

	PassRegistry passRegistry;
	ForwardPass forwardPass;
	std::string activePass = "Forward";

	DrawList drawList;

private:
	//UTILITY ---

	bool constructDrawList(); //
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex); //

	void createSwapchain();
	void createSwapchainImageViews();
	void createDepthResources();
	void createMSAAResources();
	void createFramebuffers();
	void cleanupSwapChain(); //
	void recreateSwapChain();

	void createRenderPass();

	VkDescriptorPool createDescriptorPool(uint16_t frameCount);

	void allocateGlobalDescriptorSets(uint32_t frameCount);
	void writeGlobalDescriptorSets(uint32_t frameCount);

	void createFrameContext();
};
