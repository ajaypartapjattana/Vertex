#pragma once

#include <cstdint>

namespace bitops {

	inline uint32_t ctz(uint64_t v) {
#if defined(_MSC_VER)
		unsigned long idx;
		return _BitScanForward64(&idx, v) ? idx : 64;
#else
		return v ? __builtin_ctzll(v) : 64;
#endif
	}

}