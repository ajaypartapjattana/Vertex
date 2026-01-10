#pragma once

#include "Signboard/RHI/vulkan/VulkanCommandBuffer.h"
#include "Signboard/RHI/vulkan/VulkanImage.h"
#include "Signboard/RHI/vulkan/VulkanSemaphore.h"

#include "Signboard/RHI/VulkanRHI.h"
#include "Signboard/resources/ResourceAPI.h"

#include "RenderGraph/RenderGraph.h"

#include <array>
#include <vector>
#include <memory>

static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

struct RenderQueue {
	std::vector<ObjectHandle> drawList;
};

class VulkanDevice;

class VulkanFrameBuffer;
class VulkanRenderPass;

class Renderer {
public:
	explicit Renderer(RHIView& HInterface, ResourceView resources, SceneView& scene);
	~Renderer();

	void beginFrame();
	void submitScene(const SceneView& scene);
	void renderFrame();
	void endFrame();

	void resize();

private:
	void buildGraph();
	void drawScene(CommandList& cmd);
	void drawUI(CommandList& cmd);

private:
	RHIView HInterface;
	ResourceView resources;
	SceneView scene;

	RenderGraph graph;

	uint32_t currentFrameIndex;
	uint32_t acquiredImageIndex;

	bool graphDirty = true;

	FrameUnions frameData;
	BufferHandle frameUniformBuffer;

	pass forwardPass;
	pass uiPass;

	struct Frame {
		VulkanCommandBuffer cmd;
		VulkanSemaphore imageAvailable;
		VulkanSemaphore renderFinished;
	};

	std::vector<Frame> frames;

}