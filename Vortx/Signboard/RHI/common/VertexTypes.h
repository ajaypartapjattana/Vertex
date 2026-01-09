#pragma once

#include "base/Flag_type.h"

#include <vector>

enum class VertexFormat {
	Float2,
	Float3,
	Float4,
	Uint4
};

struct VertexAttributeDesc {
	uint32_t location;
	VertexFormat format;
	uint32_t offset;
};

struct VertexBindingDesc {
	uint32_t binding;
	uint32_t stride;
	bool perinstance = false;
};

struct VertexLayoutDesc {
	std::vector<VertexAttributeDesc> attributes;
	std::vector<VertexBindingDesc> bindings;
};