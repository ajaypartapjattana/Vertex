#pragma once

#include "RenderGraphTypes.h"

class VulkanCommandBuffer;

class RenderGraph {
public:
	RenderGraph() = default;

	Pass& addPass(const RenderPassDesc&);

	void compile();
	void execute(VulkanCommandBuffer& CMDGraphics);
	void reset();

};