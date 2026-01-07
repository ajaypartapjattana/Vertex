#pragma once

#include "renderSystem/RHI/forward/vulkanFWD.h"

class Texture {
public:
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(Texture&&) noexcept;
	Texture& operator=(Texture&&) noexcept;

	~Texture() noexcept;

	VulkanImage& getImage() const { return *image; }

private:
	friend class TextureSystem;

	explicit Texture(std::unique_ptr<VulkanImage> image);

private:
	std::unique_ptr<VulkanImage> image;
	uint32_t bindlessIndex;
};
