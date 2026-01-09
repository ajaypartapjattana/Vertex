#pragma once

#include "Signboard/resources/common/TextureSystemType.h"

#include <vector>
#include <memory>

class Texture;

class VulkanDevice;
class VulkanCommandBuffer;

class VulkanDescriptorPool;
class VulkanDescriptorSet;
class VulkanDescriptorSetLayout;

class TextureSystem {
public:
	explicit TextureSystem(VulkanDevice& device, VulkanDescriptorSet& textureSet, uint32_t textureBindingIndex, uint32_t maxTextureCount);
	~TextureSystem();

	TextureHandle createTexture(VulkanCommandBuffer& cmd, const TextureDesc& desc);
	const Texture& get(TextureHandle texture) const;

	void destroy(TextureHandle texture);
	void flushDeletes();

private:
	void writeTextureDescriptor(const Texture& texture);
	void clearTextureDescriptor(const Texture& texture);

	TextureHandle assignSlot(std::unique_ptr<Texture> texture);

private:
	VulkanDevice& device;

	VulkanDescriptorSet& textureSet;

	struct Slot {
		std::unique_ptr<Texture> texture;
		uint32_t generation;
	};

	uint32_t textureBindingIndex;
	uint32_t maxTextureCount;
		
	std::vector<Slot> slots;
	std::vector<uint32_t> freeList;

	std::vector<uint32_t> pendingDeletes;

};
