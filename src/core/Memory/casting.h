#pragma once

template <typename To, typename From>
inline To bit_cast(const From& src) noexcept {
#if defined(__cpp_lib_bit_cast)
	return std::bit_case<To>(src);
#elif defined(_MSC_VER)
	return std::_Bit_cast<To>(src);
#else
	static_assert(sizeof(To) == sizeof(From), "Size mismatch");
	To dst;
	std::memcpy(&dst, &src, sizeof(To));
	return dst;
#endif
}