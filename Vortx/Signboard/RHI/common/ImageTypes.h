#pragma once

#include "base/Flag_type.h"

struct ImageExtent2D {
	uint32_t width;
	uint32_t height;
};

enum class ImageFormat {
	RGBA8,
	BGRA8,
	RGBA16F,
	Depth24Stencil8,
	Depth32F
};

enum class ImageLayout {
	Undefined,
	TransferDst,
	TransferSrc,
	ShaderReadOnly,
	ColorAttachment,
	DepthStencilAttachment,
	Present
};

enum class ImageUsage {
	ColorAttachment			= 1 << 0,
	DepthAttachment			= 1 << 1,
	Sampled					= 1 << 2
};

using ImageUsageFlags = Flags<ImageUsage>;

struct ImageDesc {
	uint32_t width;
	uint32_t height;
	ImageUsageFlags usage = ImageUsage::ColorAttachment;
	ImageFormat format = ImageFormat::RGBA8;
};