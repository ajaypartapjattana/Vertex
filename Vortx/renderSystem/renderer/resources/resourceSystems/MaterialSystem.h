#pragma once

#include "renderSystem/renderer/resources/common/MaterialSystemTypes.h"

#include "renderSystem/RHI/vulkan/VulkanBuffer.h"

#include <glm/glm.hpp>
#include <memory>

struct MaterialSlot {
	PipelineHandle pipleine;
	GPUMaterial material;
	uint32_t generation = 0;
	bool alive = false;
};

class VulkanDevice;
class VulkanDescriptorSet;
class VulkanCommandBuffer;

class PipelineSystem;

class MaterialSystem {
public:
	explicit MaterialSystem(VulkanDevice& device, VulkanDescriptorSet& materialSet, uint32_t materialBindingIndex, uint32_t maxMaterialCount);
	~MaterialSystem();

	MaterialHandle createMaterial(const MaterialDesc& desc);

	const MaterialSlot& get(MaterialHandle handle) const;

	void destroy(MaterialHandle handle);
	void flushDeletes();

private:
	MaterialHandle allocateSlot();
	void upload(uint32_t index);

	VulkanBuffer createMaterialStorageBuffer();
	void writeMaterialDescriptor();

private:
	VulkanDevice& device;

	VulkanDescriptorSet& materialSet;
	uint32_t materialBindingIndex;
	uint32_t maxMaterialCount;

	VulkanBuffer materialBuffer;

	std::vector<MaterialSlot> slots;
	std::vector<uint32_t> freeList;

	std::vector<uint32_t> pendingDeletes;
};