#include "TextureSystem.h"

#include "primitive/Texture.h"

#include "Signboard/RHI/vulkan/VulkanBuffer.h"
#include "Signboard/RHI/vulkan/VulkanImage.h"

#include "Signboard/RHI/vulkan/VulkanDescriptorSet.h"
#include "Signboard/RHI/vulkan/VulkanDescriptorWriter.h"

#include <stdexcept>
#include <cassert>

TextureSystem::TextureSystem(VulkanDevice& device, VulkanDescriptorSet& textureSet, uint32_t textureBindingIndex, uint32_t maxTextureCount)
	: device(device), textureSet(textureSet), textureBindingIndex(textureBindingIndex), maxTextureCount(maxTextureCount) {}

TextureSystem::~TextureSystem() {
	slots.clear();
	freeList.clear();
}

TextureHandle TextureSystem::createTexture(VulkanCommandBuffer& cmd, const TextureDesc& desc) {
	if (slots.size() >= maxTextureCount) {
		throw std::runtime_error("Out of bindless texture slots");
	}

	uint64_t pixelBufferSize = desc.pixelCount * desc.pixelSize;

	BufferDesc stagingPixelDesc{};
	stagingPixelDesc.size = pixelBufferSize;
	stagingPixelDesc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);
	stagingPixelDesc.usageFlags = BufferUsage::TransferSource;

	VulkanBuffer stagingPixel(device, stagingPixelDesc);
	stagingPixel.upload(desc.p_pixelData, pixelBufferSize);

	ImageDesc imageDesc{};
	imageDesc.width = desc.width;
	imageDesc.height = desc.height;
	imageDesc.format = desc.format;
	imageDesc.usage = desc.usage;

	VulkanImage textureImage(device, imageDesc);

	textureImage.copyFromBuffer(cmd, stagingPixel);
	textureImage.transitionLayout(cmd, ImageLayout::ShaderReadOnly, PipelineStage::Transfer, PipelineStage::FragmentShader);

	auto texture = std::make_unique<Texture>(textureImage);

	TextureHandle handle =  assignSlot(std::move(texture));

	const Texture& tex = get(handle);
	writeTextureDescriptor(tex);

	return handle;
}

void TextureSystem::writeTextureDescriptor(const Texture& texture) {
	VulkanDescriptorWriter writer(device, textureSet);

	writer.writeSampledImage(textureBindingIndex, texture.bindlessIndex, &texture.getImage());
	writer.commit();
}

void TextureSystem::clearTextureDescriptor(const Texture& texture) {
	VulkanDescriptorWriter writer(device, textureSet);

	writer.writeSampledImage(textureBindingIndex, texture.bindlessIndex, nullptr);
	writer.commit();
}

TextureHandle TextureSystem::assignSlot(std::unique_ptr<Texture> texture) {
	uint32_t index;

	if (!freeList.empty()) {
		index = freeList.back();
		freeList.pop_back();
	}
	else {
		index = static_cast<uint32_t>(slots.size());
		slots.emplace_back();
	}

	Slot& slot = slots[index];
	slot.texture = std::move(texture);
	slot.texture->bindlessIndex = index;

	return TextureHandle{ index, slot.generation };
}

const Texture& TextureSystem::get(TextureHandle handle) const {
	assert(handle.index < slots.size());

	const Slot& slot = slots[handle.index];

	assert(slot.texture && "TextureHandle refers to destroyed texture!");
	assert(slot.generation == handle.generation && "TextureHandle generation mismatch!");

	return *slot.texture;
}

void TextureSystem::destroy(TextureHandle handle) {
	uint32_t index = handle.index;
	if (index == INVALID_TEXTURE.index)
		return;
	assert(index < slots.size());

	Slot& slot = slots[index];
	if (slot.generation != handle.generation)
		return;

	pendingDeletes.push_back(index);
}

void TextureSystem::flushDeletes() {
	for (uint32_t index : pendingDeletes) {
		Slot& slot = slots[index];

		if (!slot.texture) continue;
		clearTextureDescriptor(*slot.texture);
		slot.texture.reset();
		++slot.generation;
		freeList.push_back(index);
	}
	pendingDeletes.clear();
}

