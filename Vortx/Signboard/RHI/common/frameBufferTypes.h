#pragma once

#include "base/Flag_type.h"
#include <vector>

class VulkanImage;

struct FrameBufferDesc {
	std::vector<VulkanImage*> colorAttachments;
	VulkanImage* depthAttachment = nullptr;

	uint32_t width;
	uint32_t height;
	uint32_t layers = 1;
};