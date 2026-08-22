#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>

struct BatchKey {
	uint32_t pipeline;
	uint32_t material;

	bool operator==(const BatchKey& other) const {
		return pipeline == other.pipeline && material == other.material;
	}
};

struct BatchKeyHash {
	size_t operator()(const BatchKey& k) const noexcept {
		size_t h1 = std::hash<uint32_t>{}(k.pipeline);
		size_t h2 = std::hash<uint32_t>{}(k.material);
		return h1 ^ (h2 << 1);
	}
};
