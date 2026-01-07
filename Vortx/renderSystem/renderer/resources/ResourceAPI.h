#pragma once

#include "resourceSystems/MeshSystem.h"
#include "resourceSystems/TextureSystem.h"
#include "resourceSystems/SamplerSystem.h"
#include "resourceSystems/MaterialSystem.h"

#include "resourceSystems/PipelineSystem.h"

#include "scene/ObjectSystem.h"

#include "renderSystem/RHI/vulkan/VulkanDescriptorPool.h"
#include "renderSystem/RHI/vulkan/VulkanDescriptorLayout.h"
#include "renderSystem/RHI/vulkan/VulkanDescriptorSet.h"

#include "DescriptorSchema.h"

struct MeshDesc;
struct TextureDesc;
struct SamplerDesc;
struct MaterialDesc;

class VulkanDevice;
class VulkanCommandBuffer;

class ResourceAPI {
public:
	ResourceAPI(VulkanDevice&);
	~ResourceAPI() = default;
	
	MeshHandle createMesh(VulkanCommandBuffer& cmd, const MeshDesc& desc);
	TextureHandle createTexture(VulkanCommandBuffer& cmd, const TextureDesc& desc);
	SamplerHandle createSampler(const SamplerDesc& desc);
	MaterialHandle createMaterial(const MaterialDesc& desc);
	ObjectHandle createObject(MeshHandle mesh, MaterialHandle material, const glm::mat4& transform);

	void destroy(MeshHandle mesh);
	void destroy(TextureHandle texture);
	void destroy(SamplerHandle sampler);
	void destroy(MaterialHandle material);
	void destory(ObjectHandle object);

	void flush();

private:
	VulkanDescriptorPool createDescriptorPool();

	VulkanDescriptorSetLayout createViewStateLayout();
	VulkanDescriptorSetLayout createObjectStateLayout();
	VulkanDescriptorSetLayout createMaterialVariableLayout();
	VulkanDescriptorSetLayout createBindlessTextureLayout();

private:
	VulkanDevice& device;

	VulkanDescriptorPool descriptorPool;

	VulkanDescriptorSetLayout viewStateLayout;
	VulkanDescriptorSetLayout objectStateLayout;
	VulkanDescriptorSetLayout materialVariableLayout;
	VulkanDescriptorSetLayout bindlessTextureLayout;

	VulkanDescriptorSet viewStateSet;
	VulkanDescriptorSet objectStateSet;
	VulkanDescriptorSet materialVariableSet;
	VulkanDescriptorSet bindlessTextureSet;

	ObjectSystem objectSystem;

	PipelineSystem pipelineSystem;

	MaterialSystem materialSystem;
	TextureSystem textureSystem;
	SamplerSystem samplerSystem;
	MeshSystem meshSystem;

};