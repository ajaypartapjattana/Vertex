#pragma once

#include "Signboard/RHI/common/ImageTypes.h"
#include "Signboard/RHI/Common/RenderPassTypes.h"

#include <vector>
#include <string>

struct PassImage{
	std::string name;
	ImageFormat format;
	ImageUsage usage;
};

struct Pass {
	std::string name;
	//PassType type;
	std::vector<std::string> colorAttachment;
	std::vector<std::string> depthAttachment;
	std::string pipeline;
};

struct RenderRoutine {
	std::string name;
	std::vector<PassImage> images;
	std::vector<Pass> passes;
};