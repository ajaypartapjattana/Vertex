#include "materials.h"

#include <array>

MaterialSystem::MaterialSystem(VkDevice device, VkDescriptorPool descriptorPool, TextureSystem& S_textures)
    : device(device), 
      descriptorPool(descriptorPool), 
      textureSystem(S_textures) 
{
}

void MaterialSystem::registerPass(PassID passID, VkDescriptorSetLayout materialLayout) {
    materialSetLayouts[passID] = materialLayout;
}

MaterialSystem::~MaterialSystem() {

}

MaterialID MaterialSystem::registerMaterial(const MaterialDesc& desc) {
    auto it = nameToID.find(desc.name);
    if (it != nameToID.end()) {
        return it->second;
    }

    MaterialID id = materials.size();

    auto material = std::make_unique<Material>();
    material->name = desc.name;
    material->pipeline = desc.pipeline;
    material->albedo = desc.albedo;
    material->normal = desc.normal;
    material->params = desc.params;

    allocateMaterialDescriptorSet(*material, desc.passID);
    writeMaterialDescriptorSet(*material, desc.passID);

    materials.push_back(std::move(material));
    nameToID[desc.name] = id;

    return id;
}

void MaterialSystem::allocateMaterialDescriptorSet(Material& material, PassID passID) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &materialSetLayouts[passID];

    vkAllocateDescriptorSets(device, &allocInfo, &material.descriptors[passID]);
}

void MaterialSystem::writeMaterialDescriptorSet(Material& material, PassID passID) {
    Texture* albedoTex = textureSystem.get(material.albedo);
    Texture* normalTex = textureSystem.get(material.normal);

    VkDescriptorImageInfo albedoInfo{};
    albedoInfo.sampler = albedoTex->getSampler();
    albedoInfo.imageView = albedoTex->getImageView();
    albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo normalInfo{};
    normalInfo.sampler = normalTex->getSampler();
    normalInfo.imageView = normalTex->getImageView();
    normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writes[2]{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = material.descriptors[passID];
    writes[0].dstBinding = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].descriptorCount = 1;
    writes[0].pImageInfo = &albedoInfo;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = material.descriptors[passID];
    writes[1].dstBinding = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].descriptorCount = 1;
    writes[1].pImageInfo = &normalInfo;

    vkUpdateDescriptorSets(device, 2, writes, 0, nullptr);
}

Material* MaterialSystem::getMaterial(MaterialID id) {
    if (id >= materials.size()) return nullptr;
    return materials[id].get();
}

MaterialID MaterialSystem::getID(const std::string& name) {
    auto it = nameToID.find(name);
    if (it == nameToID.end()) return INVALID_MATERIAL;
    return it->second;
}