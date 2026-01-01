#include "Objects.h"

#include "renderSystem/RHI/vulkan/VulkanDescriptorLayout.h"
#include "renderSystem/RHI/vulkan/VulkanDescriptorPool.h"
#include "renderSystem/RHI/vulkan/VulkanDescriptorSet.h"
#include "renderSystem/RHI/vulkan/VulkanBuffer.h"
#include "renderSystem/RHI/vulkan/VulkanDescriptorWriter.h"

#include <glm/gtc/matrix_transform.hpp>
#include <array>

struct objectUniformAllocation {
	VulkanDescriptorSet descriptor;
	VulkanBuffer UBO;
	void* mapped;

	objectUniformAllocation(VulkanDevice& device, const VulkanBufferDesc& desc) : UBO(device, desc) {}
};

ObjectSystem::ObjectSystem(VulkanDevice& device, VulkanDescriptorPool* descriptorPool)
	: device(device),
	  descriptorPool(descriptorPool),
	  objectSetLayouts(device, [] {
		  DescriptorBindingDesc binding{};
		  binding.binding = 0;
		  binding.count = 1;
		  binding.stages.set(ShaderStage::VertexBit, ShaderStage::FragmentBit);
		  binding.type = DescriptorType::UniformBuffer;

		  VulkanDescriptorSetLayoutDesc layoutDesc{};
		  layoutDesc.bindings = {binding};
		  return layoutDesc;
		  }())
{

}

ObjectSystem::~ObjectSystem() {
	for (auto& objPtr : stored) {
		if (!objPtr) continue;

		RenderObject& obj = *objPtr;
		destroy(obj.id);
	}
}

ObjectID ObjectSystem::createObject(const ObjectDesc& desc) {
	ObjectID id;

	if (!slots_F.empty()) {
		id = slots_F.back();
		slots_F.pop_back();
	} else {
		id = static_cast<ObjectID>(stored.size());
		stored.emplace_back();
	}

	auto obj = std::make_unique<RenderObject>();
	obj->id = id;
	obj->meshHandle = desc.meshHandle;
	obj->materialID = desc.materialID;

	allocateObjectDescriptorSet(*obj);
	writeObjectDescriptorSet(*obj);

	stored[id] = std::move(obj);
	count++;

	return id;
}

RenderObject* ObjectSystem::get(ObjectID id) {
	if (id == INVALID_OBJECT) return nullptr;
	if (id >= stored.size()) return nullptr;
	return stored[id].get();
}

void ObjectSystem::destroy(ObjectID id) {
	if (id >= stored.size()) return;
	if (!stored[id]) return;

	auto& obj = stored[id];

	obj.reset();
	slots_F.push_back(id);
}

void ObjectSystem::allocateObjectDescriptorSet(RenderObject& obj) {
	std::vector<objectUniformAllocation>& allocations = obj.perFrameDescriptor;
	allocations.clear();
	allocations.reserve(frameCount);

	VulkanBufferDesc UbDesc{};
	UbDesc.size = sizeof(Object_UBO);
	UbDesc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);
	UbDesc.usageFlags = BufferUsage::Uniform;

	for (uint8_t i = 0; i < frameCount; i++) {
		objectUniformAllocation& allocation = allocations.emplace_back(device, UbDesc);
		allocation.descriptor = descriptorPool->allocate(objectSetLayouts);
		allocation.mapped = allocation.UBO.map();
	}
}

void ObjectSystem::writeObjectDescriptorSet(RenderObject& obj) {
	for (uint8_t i = 0; i < frameCount; i++) {
		objectUniformAllocation& allocation = obj.perFrameDescriptor[i];

		VulkanDescriptorWriter uboWriter(device, allocation.descriptor);
		uboWriter.writeBuffer(0, allocation.UBO, DescriptorType::UniformBuffer, 0, sizeof(Object_UBO));
		uboWriter.commit();
	}
}

void ObjectSystem::updateObject_UBO(ObjectID id) {
	auto* obj = get(id);
	if (!obj) return;
	if (!obj->tranformDirty) return;

	ObjectParams& params = obj->params;

	glm::mat4 T = glm::translate(glm::mat4(1.0f), params.position);
	glm::mat4 R =
		glm::rotate(glm::mat4(1.0f), glm::radians(params.rotation.z), { 0,0,1 }) *
		glm::rotate(glm::mat4(1.0f), glm::radians(params.rotation.y), { 0,1,0 }) *
		glm::rotate(glm::mat4(1.0f), glm::radians(params.rotation.x), { 1,0,0 });
	glm::mat4 S = glm::scale(glm::mat4(1.0f), params.scale);

	Object_UBO ubo{};
	ubo.model = T * R * S;

	std::vector<objectUniformAllocation>& alloccations = obj->perFrameDescriptor;
	for (objectUniformAllocation& allocation : alloccations) {
		memcpy(allocation.mapped, &ubo, sizeof(Object_UBO));
	}
	obj->tranformDirty = false;
}