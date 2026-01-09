#pragma once
#include <type_traits>

template<typename Enum>
class Flags {
	static_assert(std::is_enum_v<Enum>, "Flags<T> requires enum");

public:
	using Underlying = std::underlying_type_t<Enum>;

	constexpr Flags() : value(0) {}
	constexpr Flags(Enum e) : value(static_cast<Underlying>(e)) {}
	explicit constexpr Flags(Underlying v) : value(v) {}

	constexpr bool has(Enum e) const {
		return (value & static_cast<Underlying>(e)) != 0;
	}

	constexpr void set(Enum e) {
		value |= static_cast<Underlying>(e);
	}

	template<typename... Enums>
	constexpr void set(Enum first, Enums... rest) {
		set(first);
		(set(rest), ...);
	}

	constexpr Underlying raw() const { return value; }

	friend constexpr Flags operator|(Flags a, Flags b) {
		return Flags(a.value | b.value);
	}

	friend constexpr Flags operator&(Flags a, Flags b) {
		return Flags(a.value & b.value);
	}

	friend constexpr Flags operator~(Flags a) {
		return Flags(~a.value);
	}

private:
	Underlying value;
};
