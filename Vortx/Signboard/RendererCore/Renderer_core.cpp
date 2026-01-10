#include "Renderer.h"

Renderer::Renderer(RHIView& HInterface, ResourceView resources, SceneView& scene)
	: HInterface(HInterface), resources(resources), scene(scene)
{
	frames.resize(FRAMES_IN_FLIGHT);
}

void Renderer::beginFrame() {
	Frame& currentFrame = frames[currentFrameIndex];

	SwapchainImageAcquire acquire =  HInterface.swapchain.accquireNextImage(currentFrame.imageAvailable);

	if (acquire.result == SwapchianAcquireResult::OutOfDate || acquire.result == SwapchianAcquireResult::SurfaceLost) {
		resize();
		return;
	}

	acquiredImageIndex = acquire.imageIndex;

	currentFrame.cmd.begin();
}

void Renderer::renderFrame() {
	
}

void Renderer::endFrame() {
	Frame& currentFrame = frames[currentFrameIndex];

	currentFrame.cmd.end();

	currentFrame.cmd.submit(currentFrame.imageAvailable, currentFrame.renderFinished);
	HInterface.swapchain.present(acquiredImageIndex, currentFrame.renderFinished);

	currentFrameIndex = (currentFrameIndex + 1) % FRAMES_IN_FLIGHT;
}