#include "Renderer_d.h"

bool Renderer::beginFrame() {
	vkWaitForFences(device, 1, &frames[currentFrame].inFlight_Fence, VK_TRUE, UINT64_MAX);

	if (swapChainExtent.width == 0 || swapChainExtent.height == 0) {
		glfwWaitEvents();
		return false;
	}

	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, frames[currentFrame].imageAvailable_Semaphore, VK_NULL_HANDLE, &accquiredImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		std::cout << "window recreated" << std::endl;
		recreateSwapChain();
		//transformController.setScreenDimensions(glm::ivec2(appContext.swapChainExtent.width, appContext.swapChainExtent.height));
		//camera.setAspectRatio(appContext.swapChainExtent.width / (float)appContext.swapChainExtent.height);
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	if (imagesInFlight[accquiredImageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(device, 1, &imagesInFlight[accquiredImageIndex], VK_TRUE, UINT64_MAX);
	}
	imagesInFlight[accquiredImageIndex] = frames[currentFrame].inFlight_Fence;

	vkResetFences(device, 1, &frames[currentFrame].inFlight_Fence);
	vkResetCommandBuffer(frames[currentFrame].commandBuffer, 0);

	return true;
}

void Renderer::drawFrame(const CameraData& global) {
	if (constructDrawList()) {
		forwardPass.build(systemView, drawList);
	}
	updateGlobal_UBO(global);
	recordCommandBuffer(frames[currentFrame].commandBuffer, accquiredImageIndex);
}

void Renderer::endFrame() {
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { frames[currentFrame].imageAvailable_Semaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &frames[currentFrame].commandBuffer;

	VkSemaphore signalSemaphores[] = { frames[currentFrame].renderFinished_Semaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;


	if (vkQueueSubmit(activeContext.graphicsQueue, 1, &submitInfo, frames[currentFrame].inFlight_Fence) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &accquiredImageIndex;

	VkResult result = vkQueuePresentKHR(activeContext.presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
		//transformController.setScreenDimensions(glm::ivec2(appContext.swapChainExtent.width, appContext.swapChainExtent.height));
		//camera.setAspectRatio(appContext.swapChainExtent.width / (float)appContext.swapChainExtent.height);
		return;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer");
	}

	PassID activePassID = passRegistry.getID(activePass);

	PassContext passInfo{};
	passInfo.passID = activePassID;
	passInfo.renderPass = renderPass;
	passInfo.framebuffer = swapChainFramebuffers[imageIndex];
	passInfo.globalSet = frames[currentFrame].globalDescriptors.at(activePassID);
	passInfo.extent = swapChainExtent;

	forwardPass.record(commandBuffer, systemView, passInfo, currentFrame);

	/*PushConstants pc{};
	pc.useTexture = (sceneRenderState.enableTextures) ? 1 : 0;
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pc);*/

	/*gui.endFrame(appContext.commandBuffers[currentFrame]);*/

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

bool Renderer::constructDrawList() {
	if (!drawList.dirty) return false;

	drawList.items.clear();
	drawList.dirty = false;

	drawList.items.reserve(objectSystem.count);
	for (auto& objPtr : objectSystem.stored) {
		if (!objPtr) continue;

		const RenderObject& obj = *objPtr;
		drawList.items.push_back(obj.id);
	}

	return true;
}