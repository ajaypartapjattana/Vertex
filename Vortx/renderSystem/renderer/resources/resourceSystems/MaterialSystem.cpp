#include "MaterialSystem.h"

#include "renderSystem/RHI/vulkan/VulkanDescriptorWriter.h"
#include "renderSystem/RHI/vulkan/VulkanDescriptorSet.h"

#include "renderSystem/RHI/vulkan/VulkanImage.h"

#include "TextureSystem.h"
#include "primitive/Texture.h"

#include <array>
#include <stdexcept>

MaterialSystem::MaterialSystem(VulkanDevice& device, VulkanDescriptorSet& materialSet, uint32_t materialBindingIndex, uint32_t maxMaterialCount)
    : device(device), materialSet(materialSet), materialBindingIndex(materialBindingIndex), maxMaterialCount(maxMaterialCount), materialBuffer(createMaterialStorageBuffer())
{
    writeMaterialDescriptor();
}

VulkanBuffer MaterialSystem::createMaterialStorageBuffer() {
    BufferDesc bufferDesc{};
    bufferDesc.size = sizeof(GPUMaterial) * maxMaterialCount;
    bufferDesc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);
    bufferDesc.usageFlags = BufferUsage::Storage;

    return VulkanBuffer(device, bufferDesc);
}

void MaterialSystem::writeMaterialDescriptor() {
    VulkanDescriptorWriter writer(device, materialSet);

    writer.writeStorageBuffer(materialBindingIndex, &materialBuffer, materialBuffer.getSize());
    writer.commit();
}

MaterialSystem::~MaterialSystem() {
}

MaterialHandle MaterialSystem::createMaterial(const MaterialDesc& desc) {
    MaterialHandle handle = allocateSlot();

    MaterialSlot& slot = slots[handle.index];
    slot.pipleine = desc.pipeline;
    slot.material = desc.material;
    slot.generation++;
    slot.alive = true;

    return handle;
}

void MaterialSystem::upload(uint32_t index) {
    materialBuffer.upload(&slots[index].material, sizeof(GPUMaterial), index * sizeof(GPUMaterial));
}

MaterialHandle MaterialSystem::allocateSlot() {
    uint32_t index;

    if (!freeList.empty()) {
        index = freeList.back();
        freeList.pop_back();
    } else {
        index = static_cast<uint32_t>(slots.size());
        slots.emplace_back();
    }

    return MaterialHandle{ index, slots[index].generation };
}

void MaterialSystem::destroy(MaterialHandle handle) {
    if (!handle.isValid())
        return;

    uint32_t index = handle.index;
    if (index >= slots.size())
        return;

    MaterialSlot& slot = slots[index];
    if (slot.generation != handle.generation || !slot.alive)
        return;

    slot.alive = false;
    pendingDeletes.push_back(index);
}

void MaterialSystem::flushDeletes() {
    for (uint32_t index : pendingDeletes) {
        MaterialSlot& slot = slots[index];

        slot.generation++;
        freeList.push_back(index);
    }
    pendingDeletes.clear();
}

const MaterialSlot& MaterialSystem::get(MaterialHandle handle) const {
    if (!handle.isValid())
        throw std::runtime_error("invalid handle!");

    uint32_t index = handle.index;
    if (index >= slots.size())
        return;

    const MaterialSlot& slot = slots[index];
    if (slot.generation != handle.generation || !slot.alive)
        return;

    return slot;
}