#pragma once

#include "renderSystem/RHI/common/ImageTypes.h"

struct TextureDesc {
	const void* p_pixelData;
	size_t pixelSize;
	size_t pixelCount;

	uint32_t width;
	uint32_t height;

	ImageFormat format;
	ImageUsage usage;
};

struct TextureHandle {
	uint32_t index;
	uint32_t generation;
};
constexpr TextureHandle INVALID_TEXTURE{ UINT32_MAX, UINT32_MAX };