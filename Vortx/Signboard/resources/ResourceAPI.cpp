#include "ResourceAPI.h"

ResourceAPI::ResourceAPI(VulkanDevice& device)
	: device(device),

	  descriptorPool(createDescriptorPool()),

	  viewStateLayout(createViewStateLayout()),
	  objectStateLayout(createObjectStateLayout()),
	  materialVariableLayout(createMaterialVariableLayout()),
	  bindlessTextureLayout(createBindlessTextureLayout()),

	  viewStateSet(descriptorPool.allocate(viewStateLayout, nullptr)),
	  objectStateSet(descriptorPool.allocate(objectStateLayout, nullptr)),
	  materialVariableSet(descriptorPool.allocate(materialVariableLayout, nullptr)),
	  bindlessTextureSet(descriptorPool.allocate(bindlessTextureLayout, DESCRIPTOR_SCHEMA::BINDLESS_TEXTURES::VARIABLE_COUNT)),

	  objectSystem(device, objectStateSet, DESCRIPTOR_SCHEMA::OBJECT_STATE::OBJECT_BUFFER_BINDING, DESCRIPTOR_SCHEMA::OBJECT_STATE::MAX_OBJECT_COUNT),
	  viewStateSystem(device, viewStateSet, DESCRIPTOR_SCHEMA::VIEW_STATE::VIEW_STATE_BINDING),

	  materialSystem(device, materialVariableSet, DESCRIPTOR_SCHEMA::MATERIAL_VARIABLES::MATERIAL_BUFFER_BINDING, DESCRIPTOR_SCHEMA::MATERIAL_VARIABLES::MAX_MATERIAL_COUNT),
	  textureSystem(device, bindlessTextureSet, DESCRIPTOR_SCHEMA::BINDLESS_TEXTURES::TEXTURE_BINDING, DESCRIPTOR_SCHEMA::BINDLESS_TEXTURES::MAX_TEXTURES),
	  samplerSystem(device, bindlessTextureSet, DESCRIPTOR_SCHEMA::BINDLESS_TEXTURES::SAMPLER_BINDING, DESCRIPTOR_SCHEMA::BINDLESS_TEXTURES::MAX_SAMPLERS),
	  pipelineSystem(device),
	  meshSystem(device)
{

}

VulkanDescriptorPool ResourceAPI::createDescriptorPool() {
	DescriptorPoolSizeDesc UBO_pool{};
	UBO_pool.type = DescriptorType::UniformBuffer;
	UBO_pool.count = 512;

	DescriptorPoolSizeDesc Texture_pool{};
	Texture_pool.type = DescriptorType::SampledImage;
	Texture_pool.count = 512;

	DescriptorPoolSizeDesc Sampler_pool{};
	Sampler_pool.type = DescriptorType::TextureSampler;
	Sampler_pool.count = 32;

	DescriptorPoolDesc desc;
	desc.poolSizes = { UBO_pool, Texture_pool, Sampler_pool };
	desc.maxSets = 128;

	return VulkanDescriptorPool(device, desc);
}

VulkanDescriptorSetLayout ResourceAPI::createViewStateLayout() {
	DescriptorBindingDesc viewState{};
	viewState.binding = DESCRIPTOR_SCHEMA::VIEW_STATE::VIEW_STATE_BINDING;
	viewState.type = DescriptorType::UniformBuffer;
	viewState.stages.set(ShaderStageBit::VertexBit, ShaderStageBit::FragmentBit);
	viewState.count = DESCRIPTOR_SCHEMA::VIEW_STATE::VIEW_STATE_COUNT;
	
	DescriptorSetLayoutDesc desc{};
	desc.bindings = { viewState };
	
	return VulkanDescriptorSetLayout(device, desc);
}

VulkanDescriptorSetLayout ResourceAPI::createObjectStateLayout() {
	DescriptorBindingDesc objectState{};
	objectState.binding = DESCRIPTOR_SCHEMA::OBJECT_STATE::OBJECT_BUFFER_BINDING;
	objectState.type = DescriptorType::StorageBuffer;
	objectState.stages.set(ShaderStageBit::VertexBit);
	objectState.count = DESCRIPTOR_SCHEMA::OBJECT_STATE::MAX_OBJECT_COUNT;

	DescriptorSetLayoutDesc desc{};
	desc.bindings = { objectState };

	return VulkanDescriptorSetLayout(device, desc);
}

VulkanDescriptorSetLayout ResourceAPI::createMaterialVariableLayout() {
	DescriptorBindingDesc MaterialParams{};
	MaterialParams.binding = DESCRIPTOR_SCHEMA::MATERIAL_VARIABLES::MATERIAL_BUFFER_BINDING;
	MaterialParams.type = DescriptorType::StorageBuffer;
	MaterialParams.stages.set(ShaderStageBit::VertexBit, ShaderStageBit::FragmentBit);
	MaterialParams.count = DESCRIPTOR_SCHEMA::MATERIAL_VARIABLES::MAX_MATERIAL_COUNT;

	DescriptorSetLayoutDesc desc{};
	desc.bindings = { MaterialParams };

	return VulkanDescriptorSetLayout(device, desc);
}

VulkanDescriptorSetLayout ResourceAPI::createBindlessTextureLayout() {
	DescriptorBindingDesc textures{};
	textures.binding = DESCRIPTOR_SCHEMA::BINDLESS_TEXTURES::TEXTURE_BINDING;
	textures.type = DescriptorType::SampledImage;
	textures.stages.set(ShaderStageBit::FragmentBit);
	textures.count = DESCRIPTOR_SCHEMA::BINDLESS_TEXTURES::MAX_TEXTURES;
	textures.flags.set(DescriptorBindingBit::PatiallyBound, DescriptorBindingBit::UpdateAfterBind, DescriptorBindingBit::VariableCount);

	DescriptorBindingDesc samplers{};
	samplers.binding = DESCRIPTOR_SCHEMA::BINDLESS_TEXTURES::SAMPLER_BINDING;
	samplers.type = DescriptorType::TextureSampler;
	samplers.stages.set(ShaderStageBit::FragmentBit);
	samplers.count = DESCRIPTOR_SCHEMA::BINDLESS_TEXTURES::MAX_SAMPLERS;
	samplers.flags.set(DescriptorBindingBit::PatiallyBound, DescriptorBindingBit::UpdateAfterBind, DescriptorBindingBit::VariableCount);

	DescriptorSetLayoutDesc desc{};
	desc.bindings = { textures, samplers };

	return VulkanDescriptorSetLayout(device, desc);
}

MeshHandle ResourceAPI::createMesh(VulkanCommandBuffer& cmd, const MeshDesc& desc) {
	return meshSystem.createMesh(cmd, desc);
}

ResourceView ResourceAPI::getResourceView() {
	return ResourceView{ 
		pipelineSystem, 
		materialSystem, 
		textureSystem, 
		samplerSystem, 
		meshSystem 
	};
}

SceneView ResourceAPI::getSceneView() {
	return SceneView{
		objectSystem,
		viewStateSystem,
	};
}

