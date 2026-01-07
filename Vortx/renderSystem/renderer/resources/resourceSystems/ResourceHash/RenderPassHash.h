#pragma once

#include "HashBase/Hash.h"
#include "renderSystem/RHI/common/RenderPassTypes.h"

struct RenderPassKey {
	std::vector<ImageFormat> colorFormats;
	bool hasDepth;
	ImageFormat depthFormat;

	bool operator==(const RenderPassKey& rhs) const {
		return colorFormats == rhs.colorFormats &&
			hasDepth == rhs.hasDepth &&
			depthFormat == rhs.depthFormat;
	}
};

struct RenderPassKeyHash {
	size_t operator()(const RenderPassKey& k) const {
		size_t hash = 0;
		hashCombineRange(hash, k.colorFormats.data(), k.colorFormats.size());
		hashCombine(hash, k.hasDepth);
		hashCombine(hash, k.depthFormat);

		return hash;
	}
};