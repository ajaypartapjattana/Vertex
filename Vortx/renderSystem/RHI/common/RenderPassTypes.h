#pragma once

#include "ImageTypes.h"
#include <vector>

enum class LoadOp {
	Load,
	Clear,
	DontCare
};

enum class StoreOp {
	Store,
	DontCare
};

struct AttachmentDesc {
	ImageFormat format;
	LoadOp load;
	StoreOp store;
};

struct RenderPassDesc {
	std::vector<AttachmentDesc> colorAttachments;
	bool hasDepth = false;
	AttachmentDesc depthAttachment;
};

