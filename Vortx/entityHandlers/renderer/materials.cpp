#include "materials.h"

MaterialSystem::MaterialSystem(VkDevice device, VkDescriptorPool descriptorPool) : device(device), descriptorPool(descriptorPool) {

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
    material->params = desc.params;
    material->pipeline = desc.pipeline;

    materials.push_back(std::move(material));
    nameToID[desc.name] = id;

    return id;
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