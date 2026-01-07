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

enum class PipelineStage {
	TopOfPipe				= 1 << 0,
	Transfer				= 1 << 1,
	VertexInput				= 1 << 2,
	VertexShader			= 1 << 3,
	FragmentShader			= 1 << 4,
	EarlyDepthTest			= 1 << 5,
	LateDepthTest			= 1 << 6,
	ColorAttachmentOutput	= 1 << 7,
	BottomOfPipe			= 1 << 8
};

using PipelineStageFlags = Flags<PipelineStage>;

struct ImageDesc {
	uint32_t width;
	uint32_t height;
	ImageUsageFlags usage = ImageUsage::ColorAttachment;
	ImageFormat format = ImageFormat::RGBA8;
};