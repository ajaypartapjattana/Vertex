#pragma once

#include <stdint.h>

class RenderGraph;

class RenderBackend {
public:
	virtual ~RenderBackend() = default;
	
	virtual void resize(uint32_t wodth, uint32_t height) = 0;
	virtual bool beginFrame() = 0;
	virtual void endFrame() = 0;

	virtual RenderGraph& graph() = 0;
};
