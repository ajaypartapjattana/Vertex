#pragma once

#include "pipelines.h"
#include "glm/glm.hpp"

using MaterialID = uint32_t;
static constexpr MaterialID INVALID_MATERIAL = UINT32_MAX;

struct MaterialParams {
	glm::vec4 baseColor = glm::vec4(1.0f);
	float roughness = 0.5f;
	float metallic = 0.0f;
	float IOR = 1.5f;
	float alpha = 1.0f;
};

struct MaterialDesc {
	std::string name;

	PipelineID pipeline = INVALID_PIPELINE;
	MaterialParams params;
};



struct Material {
	std::string name;

	PipelineID pipeline;
	MaterialParams params;
};

class MaterialSystem {
public:
	MaterialSystem(VkDevice device, VkDescriptorPool descriptorPool);
	~MaterialSystem();

	MaterialID registerMaterial(const MaterialDesc& desc);
	Material* getMaterial(MaterialID id);
	MaterialID getID(const std::string& name);

private:
	VkDevice device;
	VkDescriptorPool descriptorPool;

	std::vector<std::unique_ptr<Material>> materials;
	std::unordered_map<std::string, MaterialID> nameToID;
};