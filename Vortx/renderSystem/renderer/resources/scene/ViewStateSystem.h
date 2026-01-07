#pragma once

#include "renderSystem/RHI/vulkan/VulkanBuffer.h"

#include <glm/glm.hpp>

struct ViewUniform {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewProj;
	
	glm::vec3 cameraPos;
	float padding;
};

class VulkanDevice;
class VulkanDescriptorSet;
class VulkanBuffer;

class ViewStateSystem {
public:
	ViewStateSystem(VulkanDevice& device, VulkanDescriptorSet& viewStateSet, uint32_t viewStateBinding);

	void setCamera(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& position);
	void upload();

	const VulkanBuffer& buffer() const;

private:
	VulkanBuffer createViewStateBuffer();
	void writeViewStateBuffer();

private:
	VulkanDevice& device;

	VulkanBuffer viewStateBuffer;
	ViewUniform* mapped = nullptr;

	VulkanDescriptorSet& viewStateSet;
	uint32_t viewStateBinding;
};