#pragma once

#include "renderSystem/RHI/common/PipelineTypes.h"
#include "HashBase/Hash.h"

struct PipelineKey {
	PipelineType type;

	VkPipelineLayout layout;
	VkRenderPass renderPass;

	ImageFormat colorFormat;
	ImageFormat depthForamt;

	std::vector<uint64_t> shadersHashes;

	RasterState raster;
	BlendState blend;

	bool operator==(const PipelineKey& rhs) const {
		return type == rhs.type &&
			layout == rhs.layout &&
			renderPass == rhs.renderPass &&
			colorFormat == rhs.colorFormat &&
			depthForamt == rhs.depthForamt &&
			shadersHashes == rhs.shadersHashes &&
			raster.depthTest == rhs.raster.depthTest &&
			raster.depthWrite == rhs.raster.depthWrite &&
			blend.enable == rhs.blend.enable;
	}
};

struct PipelineKeyHash {
	size_t operator()(const PipelineKey& key) const {
		return hashPipelineKey(key);
	}
};

size_t hashPipelineKey(const PipelineKey& key) {
	size_t hash = 0;
	hashCombine(hash, key.type);
	hashCombine(hash, key.layout);
	hashCombine(hash, key.renderPass);
	hashCombine(hash, key.colorFormat);
	hashCombine(hash, key.depthForamt);

	hashCombineRange(hash, key.shadersHashes.data(), key.shadersHashes.size());

	hashCombine(hash, key.raster);
	hashCombine(hash, key.blend);

	return hash;
}

uint64_t hashShaderFile(const std::string& path) {
	auto bytes = readBinaryFile(path.c_str());

	return fnv1a64(bytes.data(), bytes.size());
}