#include "ObjectSystem.h"

#include "renderSystem/RHI/vulkan/VulkanDescriptorSet.h"
#include "renderSystem/RHI/vulkan/VulkanBuffer.h"
#include "renderSystem/RHI/vulkan/VulkanDescriptorWriter.h"

#include <glm/gtc/matrix_transform.hpp>
#include <array>

struct objectUniformAllocation {
	VulkanDescriptorSet descriptor;
	VulkanBuffer UBO;
	void* mapped;

	objectUniformAllocation(VulkanDevice& device, const BufferDesc& desc) : UBO(device, desc) {}
};

ObjectSystem::ObjectSystem(VulkanDevice& device, VulkanDescriptorSet& objectSet, uint32_t objectBufferBinding, uint32_t maxObjectCount)
	: device(device), objectSet(objectSet), objectBufferBinding(objectBufferBinding), maxObjectCount(maxObjectCount), objectBuffer(createObjectStoragebuffer())
{
	mapped = reinterpret_cast<GPUObject*>(objectBuffer.map());
	writeObjectStorageBuffer();
}

VulkanBuffer ObjectSystem::createObjectStoragebuffer() {
	BufferDesc desc{};
	desc.size = sizeof(GPUObject) * maxObjectCount;
	desc.usageFlags = BufferUsage::Storage;
	desc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);

	return VulkanBuffer(device, desc);
}

void ObjectSystem::writeObjectStorageBuffer() {
	VulkanDescriptorWriter writer(device, objectSet);

	writer.writeStorageBuffer(objectBufferBinding, &objectBuffer, objectBuffer.getSize());
	writer.commit();
}

ObjectSystem::~ObjectSystem() {
}

ObjectHandle ObjectSystem::createObject(MeshHandle mesh, MaterialHandle material, glm::mat4 transform) {
	ObjectHandle handle = allocateSlot();

	uint32_t index = handle.index;
	ObjectSlot& slot = slots[index];
	slot.generation++;
	slot.alive = true;

	GPUObject& gpu = mapped[index];
	gpu.meshIndex = mesh.index;
	gpu.materialIndex = material.index;
	gpu.model = transform;

	return ObjectHandle{ index, slot.generation };
}

ObjectHandle ObjectSystem::allocateSlot() {
	uint32_t index;

	if (!freeList.empty()) {
		index = freeList.back();
		freeList.pop_back();
	} else {
		index = static_cast<uint32_t>(slots.size());
		slots.emplace_back();
	}

	return ObjectHandle{ index, slots[index].generation };
}

void ObjectSystem::updateTransform(ObjectHandle handle, const glm::mat4& transform) {
	ObjectSlot& slot = slots[handle.index];
	if (!slot.alive || slot.generation != handle.generation)
		return;

	mapped[handle.index].model = transform;
}

void ObjectSystem::destroy(ObjectHandle handle) {
	ObjectSlot& slot = slots[handle.index];
	if (!slot.alive || slot.generation != handle.generation)
		return;

	slot.alive = false;
	freeList.push_back(handle.index);
}

void ObjectSystem::flushDeletes() {
	for (uint32_t index : freeList) {
		++slots[index].generation;
	}
	freeList.clear();
}

{
	/*glm::mat4 T = glm::translate(glm::mat4(1.0f), params.position);
	glm::mat4 R =
		glm::rotate(glm::mat4(1.0f), glm::radians(params.rotation.z), { 0,0,1 }) *
		glm::rotate(glm::mat4(1.0f), glm::radians(params.rotation.y), { 0,1,0 }) *
		glm::rotate(glm::mat4(1.0f), glm::radians(params.rotation.x), { 1,0,0 });
	glm::mat4 S = glm::scale(glm::mat4(1.0f), params.scale);

	ubo.model = T * R * S;*/
}