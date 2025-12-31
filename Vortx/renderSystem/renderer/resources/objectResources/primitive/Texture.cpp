#include "Texture.h"

#include "renderSystem/RHI/vulkan/VulkanImage.h"

#include <utility>

Texture::Texture(std::unique_ptr<VulkanImage> image)
	: image(std::move(image)) {}

Texture::~Texture() = default;

Texture::Texture(Texture&& other) noexcept = default;

Texture& Texture::operator=(Texture&& other) noexcept = default;

VulkanImage& Texture::getImage() const { return *image; }