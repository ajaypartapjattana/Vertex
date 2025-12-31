#include "TextureSystem.h"

#include "primitive/Texture.h"

#include "renderSystem/RHI/vulkan/VulkanBuffer.h"
#include "renderSystem/RHI/vulkan/VulkanImage.h"

#include <stdexcept>

TextureSystem::TextureSystem(VulkanDevice* device)
	: device(device) {}

TextureSystem::~TextureSystem() {
	textures.clear();
}

TextureHandle TextureSystem::addTexture(std::unique_ptr<Texture> texture) {
	TextureHandle handle;

	if (!slots_F.empty()) {
		handle = slots_F.back();
		slots_F.pop_back();
		textures[handle] = std::move(texture);
	} else {
		handle = static_cast<TextureHandle>(textures.size());
		textures.push_back(std::move(texture));
	}

	return handle;
}

Texture* TextureSystem::get(TextureHandle handle) {
	if (handle == INVALID_TEXTURE) return nullptr;
	if (handle >= textures.size()) return nullptr;
	return textures[handle].get();
}

void TextureSystem::destroy(TextureHandle handle) {
	if (handle == INVALID_TEXTURE) return ;
	if (handle >= textures.size()) return ;
	if (!textures[handle]) return;

	textures[handle].reset();
	slots_F.push_back(handle);
}

void TextureSystem::createTexture(VulkanCommandBuffer& cmd, const TextureDesc& desc) {

	uint64_t pixelBufferSize = desc.pixelCount * desc.pixelSize;

	VulkanBufferDesc stagingPixelDesc{};
	stagingPixelDesc.size = pixelBufferSize;
	stagingPixelDesc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);
	stagingPixelDesc.usageFlags = BufferUsage::TransferSource;

	VulkanBuffer stagingPixel(device, stagingPixelDesc);
	stagingPixel.upload(desc.p_pixelData, pixelBufferSize);

	VulkanImageDesc imageDesc{};
	imageDesc.width = desc.width;
	imageDesc.height = desc.height;
	imageDesc.format = ;
	imageDesc.usage = ImageUsage::ColorAttachment;

	VulkanImage textureImage(device, imageDesc);

	textureImage.copyFromBuffer(cmd, stagingPixel);
	textureImage.transitionLayout(cmd, ImageLayout::ShaderReadOnly, PipelineStage::Transfer, PipelineStage::FragmentShader);

	auto texture = std::make_unique<Texture>(textureImage);

	addTexture(std::move(texture));
}

