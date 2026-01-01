#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

struct Object_UBO
{
	glm::mat4 model;
};

struct ObjectDesc {
	uint32_t passID;
	uint32_t materialID;
	uint32_t meshHandle;
	glm::mat4 transform;
};

struct ObjectParams {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

struct objectUniformAllocation;

struct RenderObject {
	ObjectID id;

	uint32_t meshHandle;
	uint32_t materialID;

	ObjectParams params;
	bool tranformDirty = true;

	std::vector<objectUniformAllocation> perFrameDescriptor;
};

using ObjectID = uint32_t;
static constexpr ObjectID INVALID_OBJECT = UINT32_MAX;

class VulkanDevice;
class VulkanDescriptorPool;
class VulkanDescriptorSetLayout;
class VulkanDescriptorSet;
class VulkanBuffer;

class ObjectSystem {
public:
	ObjectSystem(VulkanDevice& device, VulkanDescriptorPool* descriptorPool);
	~ObjectSystem();

	ObjectID createObject(const ObjectDesc& desc);
	RenderObject* get(ObjectID id);
	void destroy(ObjectID id);

	void updateObject_UBO(ObjectID id);

	std::vector<std::unique_ptr<RenderObject>> stored;
	uint32_t count = 0;

private:
	void allocateObjectDescriptorSet(RenderObject& obj);
	void writeObjectDescriptorSet(RenderObject& obj);

private:
	VulkanDevice& device;

	VulkanDescriptorPool* descriptorPool;
	VulkanDescriptorSetLayout objectSetLayouts;

	uint8_t frameCount = 2;

	std::vector<ObjectID> slots_F;

};
