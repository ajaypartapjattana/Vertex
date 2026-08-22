#pragma once

#include <cstdint>

inline uint16_t intrin_byteSwap16(uint16_t _Value) noexcept {
#if defined(_MSC_VER)
	return _byteswap_ushort(_Value);
#elif defined(__GNUC__) || defined(__clang__)
	return __builtin_bswap16(_Value);
#else
	return uint16_t((value >> 8) | (value << 8));
#endif
}

inline uint32_t intrin_byteSwap32(uint32_t _Value) noexcept {
#if defined(_MSC_VER)
	return _byteswap_ulong(_Value);
#elif defined(__GNUC__) || defined(__clang__)
	return __builtin_bswap32(_Value);
#else
	return ((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8) | ((value & 0x00FF0000u) >> 8) | ((value & 0xFF000000u) >> 24);
#endif
}

inline uint32_t intrin_popCount32(const uint32_t _Value) noexcept {
#if defined(_MSC_VER)
	return static_cast<uint32_t>(__popcnt(_Value));
#elif defined(__GNUC__) || defined(__clang)
	return static_cast<uint32_t>(__builtin_popcount(_Value));
#else
	uint32_t value = _Value;
	uint32_t count = 0;
	while (value) {
		value &= value - 1;
		++count;
	}
	return count;
#endif
}