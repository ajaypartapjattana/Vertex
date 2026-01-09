#include "Texture.h"

#include "Signboard/RHI/vulkan/VulkanImage.h"

#include <utility>

Texture::Texture(std::unique_ptr<VulkanImage> image)
	: image(std::move(image)) {}

Texture::~Texture() = default;

Texture::Texture(Texture&& other) noexcept = default;

Texture& Texture::operator=(Texture&& other) noexcept = default;