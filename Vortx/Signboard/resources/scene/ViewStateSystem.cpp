#include "ViewStateSystem.h"

#include "renderSystem/RHI/vulkan/VulkanDescriptorWriter.h"

ViewStateSystem::ViewStateSystem(VulkanDevice& device, VulkanDescriptorSet& viewStateSet, uint32_t viewStateBinding)
	: device(device), viewStateSet(viewStateSet), viewStateBinding(viewStateBinding), viewStateBuffer(createViewStateBuffer()), mapped(reinterpret_cast<ViewUniform*>(viewStateBuffer.map()))
{
	writeViewStateBuffer();
}

VulkanBuffer ViewStateSystem::createViewStateBuffer() {
	BufferDesc desc{};
	desc.size = sizeof(ViewUniform);
	desc.usageFlags = BufferUsage::Uniform;
	desc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);

	return VulkanBuffer(device, desc);
}

void ViewStateSystem::writeViewStateBuffer() {
	VulkanDescriptorWriter writer(device, viewStateSet);

	writer.writeUniformBuffer(viewStateBinding, &viewStateBuffer, viewStateBuffer.getSize());
	writer.commit();
}

void ViewStateSystem::setCamera(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& cameraPos) {
	mapped->view = view;
	mapped->proj = proj;
	mapped->viewProj = proj * view;
	mapped->cameraPos = cameraPos;
}

void ViewStateSystem::upload() {

}