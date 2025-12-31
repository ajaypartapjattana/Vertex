#pragma once

#include <vulkan/vulkan.h>
#include <array>

#include "Vertex.h"

struct VertexLayout {
	static VkVertexInputBindingDescription binding();
	static std::array<VkVertexInputAttributeDescription, 5> attributes();
};


