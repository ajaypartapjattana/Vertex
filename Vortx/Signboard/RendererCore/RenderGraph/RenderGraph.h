#pragma once

#include "RenderGraphTypes.h"

class RenderGraph {
public:
	RenderGraph() = default;
	
	ImageHandle createImage(const ImageDesc& desc);
	BufferHandle createBuffer(const BufferDesc& desc);

	Pass& addPass(const std::string& name);
};