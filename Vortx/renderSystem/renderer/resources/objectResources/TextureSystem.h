#pragma once

#include <vector>
#include <memory>
#include <string>

struct TextureDesc {
	const void* p_pixelData;
	size_t pixelSize;
	size_t pixelCount;

	uint32_t width;
	uint32_t height;
};

using TextureHandle = uint32_t;
constexpr TextureHandle INVALID_TEXTURE = UINT32_MAX;

class Texture;

class VulkanDevice;
class VulkanCommandBuffer;

class TextureSystem {
public:
	TextureSystem(VulkanDevice* device);
	~TextureSystem();

	void createTexture(VulkanCommandBuffer& cmd, const TextureDesc& desc);
	Texture* get(TextureHandle texture);
	void destroy(TextureHandle texture);

private:
	TextureHandle addTexture(std::unique_ptr<Texture> texture);

private:

	VulkanDevice* device;
	std::vector<std::unique_ptr<Texture>> textures;
	std::vector<TextureHandle> slots_F;

};
