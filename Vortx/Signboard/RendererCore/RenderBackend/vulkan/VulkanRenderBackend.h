#pragma once

#include "renderSystem/renderer/RendererCore/RenderBackend/RenderBackend.h"

#include "renderSystem/RHI/vulkan/VulkanSwapchain.h"
#include "renderSystem/RHI/vulkan/VulkanCommandBuffer.h"
#include "renderSystem/RHI/vulkan/VulkanSemaphore.h"

class VulkanDevice;
class VulkanSurface;

class VulkanRenderBackend final : public RenderBackend {
public:
	struct FrameContext {
		VulkanCommandBuffer commandBuffer;
		VulkanSemaphore imageAvailable_Semaphore;
		VulkanSemaphore renderFinished_Semaphore;
	};

public:
	VulkanRenderBackend(VulkanDevice& device, VulkanSurface& surface, uint32_t width, uint32_t height);
	~VulkanRenderBackend();

	bool beginFrame() override;
	void executegraph();
	void endFrame() override;

	void resize(uint32_t width, uint32_t height) override;

	RenderGraph& graph() override { return renderGraph; }

private:
	void createFrameContexts();
	void recreateSwapchain();

private:
	static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

	VulkanDevice& device;

	VulkanSwapchain swapchain;
	
	std::vector<FrameContext> frames;
	uint32_t frameIndex = 0;
	uint32_t imageIndex = 0;

	RenderGraph renderGraph;

};
