#include "Objects.h"

#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include "utility/VulkanUtils.h"

ObjectSystem::ObjectSystem(VkDevice device, VkPhysicalDevice physicalDevice, VkDescriptorPool descriptorPool)
	: device(device),
	  physicalDevice(physicalDevice),
	  descriptorPool(descriptorPool) 
{
}

ObjectSystem::~ObjectSystem() {
	for (auto& objPtr : stored) {
		if (!objPtr) continue;

		RenderObject& obj = *objPtr;
		destroy(obj.id);
	}
}

void ObjectSystem::registerPass(PassID id, VkDescriptorSetLayout objectSetLayout) {
	objectSetLayouts[id] = objectSetLayout;
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
	obj->mesh = desc.mesh;
	obj->material = desc.material;

	allocateObjectDescriptorSet(*obj, desc.passID);
	writeObjectDescriptorSet(*obj, desc.passID);

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

	//vkDeviceWaitIdle(device);
	for (auto& [passID, descriptors] : obj->objectDescriptors) {
		for (uint8_t i = 0; i < frameCount; i++) {
			UniformAllocation& descriptor = descriptors[i];
			vkUnmapMemory(device, descriptor.objectMemory);
			vkDestroyBuffer(device, descriptor.UBO, nullptr);
			vkFreeMemory(device, descriptor.objectMemory, nullptr);
		}
	}

	obj.reset();
	slots_F.push_back(id);
}

void ObjectSystem::allocateObjectDescriptorSet(RenderObject& obj, PassID passID) {
	std::vector<UniformAllocation>& allocations = obj.objectDescriptors.at(passID);
	allocations.resize(frameCount);

	for (uint8_t i = 0; i < frameCount; i++) {
		UniformAllocation& allocation = allocations[i];

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &objectSetLayouts[passID];

		if (vkAllocateDescriptorSets(device, &allocInfo, &allocation.descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate object descriptor set");
		}

		VulkanUtils::createBuffer(physicalDevice, device, sizeof(Object_UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, allocation.UBO, allocation.objectMemory);

		vkMapMemory(device, allocation.objectMemory, 0, sizeof(Object_UBO), 0, &allocation.mappedPtr);
	}
}

void ObjectSystem::writeObjectDescriptorSet(RenderObject& obj, PassID passID) {
	std::vector<UniformAllocation>& allocations = obj.objectDescriptors.at(passID);

	for (uint8_t i = 0; i < frameCount; i++) {
		UniformAllocation& allocation = allocations[i];
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = allocation.UBO;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(Object_UBO);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = allocation.descriptorSet;
		write.dstBinding = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}
}

void ObjectSystem::updateObject_UBO(ObjectID id, PassID passID) {
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

	std::vector<UniformAllocation>& alloccations = obj->objectDescriptors.at(passID);
	for (UniformAllocation& allocation : alloccations) {
		memcpy(allocation.mappedPtr, &ubo, sizeof(Object_UBO));
	}
	obj->tranformDirty = false;
}