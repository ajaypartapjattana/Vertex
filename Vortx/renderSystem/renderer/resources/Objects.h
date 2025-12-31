#pragma once

#include <glm/glm.hpp>

#include "objectResources/materials.h"
#include "objectResources/MeshSystem.h"
#include "objectResources/TextureSystem.h"

struct Object_UBO
{
	glm::mat4 model;
};

struct ObjectDesc {
	PassID passID;
	MeshHandle mesh;
	MaterialID material;
	glm::mat4 transform;
};

struct ObjectParams {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

struct RenderObject {
	ObjectID id;

	MeshHandle mesh;
	MaterialID material;

	ObjectParams params;
	bool tranformDirty = true;

	std::unordered_map<PassID, std::vector<UniformAllocation>> objectDescriptors;
};

using ObjectID = uint32_t;
static constexpr ObjectID INVALID_OBJECT = UINT32_MAX;

class VulkanDevice;

class ObjectSystem {
public:
	ObjectSystem(VulkanDevice* device, VkDescriptorPool descriptorPool);
	~ObjectSystem();

	void registerPass(PassID id, VkDescriptorSetLayout objectSetLayout);

	ObjectID createObject(const ObjectDesc& desc);
	RenderObject* get(ObjectID id);
	void destroy(ObjectID id);

	void updateObject_UBO(ObjectID id, PassID passID);

	std::vector<std::unique_ptr<RenderObject>> stored;
	uint32_t count = 0;

private:
	void allocateObjectDescriptorSet(RenderObject& obj, PassID passID);
	void writeObjectDescriptorSet(RenderObject& obj, PassID passID);

private:
	VulkanDevice* device;

	VkDescriptorPool descriptorPool;

	uint8_t frameCount = 2;
	std::unordered_map<PassID, VkDescriptorSetLayout&> objectSetLayouts;

	std::vector<ObjectID> slots_F;

};
