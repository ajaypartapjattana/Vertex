#pragma once

#include <glm/glm.hpp>

#include "RenderPass/pipelines.h"

#include "TextureSystem.h"
#include "renderer/coreH/RenderTypes.h"

struct MaterialParams {
	glm::vec4 baseColor = glm::vec4(1.0f);
	float roughness = 0.5f;
	float metallic = 0.0f;
	float IOR = 1.5f;
	float alpha = 1.0f;
};

struct MaterialDesc {
	std::string name;

	PassID passID;

	PipelineID pipeline = INVALID_PIPELINE;

	TextureHandle albedo = INVALID_TEXTURE;
	TextureHandle normal = INVALID_TEXTURE;

	MaterialParams params;

};

struct Material {
	std::string name;

	PipelineID pipeline;
	std::unordered_map<PassID, VkDescriptorSet> descriptors;

	TextureHandle albedo = INVALID_TEXTURE;
	TextureHandle normal = INVALID_TEXTURE;

	MaterialParams params;
};

class MaterialSystem {
public:
	MaterialSystem(VkDevice device, VkDescriptorPool descriptorPool, TextureSystem& S_textures);
	~MaterialSystem();

	void registerPass(PassID passID, VkDescriptorSetLayout materialLayout );

	MaterialID registerMaterial(const MaterialDesc& desc);
	Material* getMaterial(MaterialID id);
	MaterialID getID(const std::string& name);

private:
	VkDevice device;
	VkDescriptorPool descriptorPool;

	//resp--
	TextureSystem& textureSystem;

	std::unordered_map<PassID, VkDescriptorSetLayout&> materialSetLayouts;

	void allocateMaterialDescriptorSet(Material& material, PassID passID);
	void writeMaterialDescriptorSet(Material& material, PassID passID);

	std::vector<std::unique_ptr<Material>> materials;
	std::unordered_map<std::string, MaterialID> nameToID;
};