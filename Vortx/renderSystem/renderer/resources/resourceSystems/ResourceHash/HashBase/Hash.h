#pragma once

#include <functional>

template<typename T>
inline void hashCombine(std::size_t& seed, const T& v) {
	seed ^= std::hash<T>{}(v)+0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

template<typename T>
inline void hashCombineRange(std::size_t& seed, const T* data, size_t count) {
	for (size_t i = 0; i < count; ++i)
		hashCombine(seed, data[i]);
}

inline uint64_t fnv1a64(const void* data, size_t size) {
	const uint8_t* bytes = static_cast<const uint8_t*>(data);

	uint64_t hash = 14695981039346656037ULL;

	for (size_t i = 0; i < size; ++i) {
		hash ^= bytes[i];
		hash *= 1099511628211ULL;
	}

	return hash;
}