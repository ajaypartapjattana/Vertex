#pragma once

#include "renderSystem/renderer/resources/common/ObjectSystemTypes.h"

#include "renderSystem/RHI/vulkan/VulkanBuffer.h"

#include <vector>
#include <memory>

class VulkanDevice;
class VulkanDescriptorPool;
class VulkanDescriptorSetLayout;
class VulkanDescriptorSet;

class ObjectSystem {
public:
	ObjectSystem(VulkanDevice& device, VulkanDescriptorSet& objectSet, uint32_t objectBufferBinding, uint32_t maxObjectBinding);
	~ObjectSystem();

	ObjectHandle createObject(MeshHandle mesh, MaterialHandle material, const glm::mat4 tranform);

	void updateTransform(ObjectHandle handle, const glm::mat4& transform);

	void destroy(ObjectHandle handle);
	void flushDeletes();

private:
	VulkanBuffer createObjectStoragebuffer();
	void writeObjectStorageBuffer();

	ObjectHandle allocateSlot();

private:
	VulkanDevice& device;

	VulkanDescriptorSet& objectSet;
	uint32_t objectBufferBinding;
	uint32_t maxObjectCount;

	VulkanBuffer objectBuffer;
	GPUObject* mapped = nullptr;

	struct ObjectSlot {
		uint32_t generation = 0;
		bool alive = false;
	};
	
	std::vector<ObjectSlot> slots;
	std::vector<uint32_t> freeList;
};
