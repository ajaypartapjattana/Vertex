#pragma once

#include <string>
#include <vector>
#include <functional>

struct RenderPassDesc {

};

struct Pass {
	std::string name;

	std::vector <ImageAccess> imageAccess;
	std::vector<BufferAccess> bufferAccess;

	std::function<void(VulkanCommandBuffer)> execute;
};