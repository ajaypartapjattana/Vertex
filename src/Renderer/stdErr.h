#pragma once

#include <system_error>

enum class RCode : int32_t {
	SUCCESS = 0,
	FAILURE,
	OUT_OF_MEMORY,
	HARDWARE_INCAPABILITY,
	SOFTWARE_INCAPABILITY,
	TIMEOUT,
	NOT_PERMITTED,
	NOT_SUPPORTED,
	INVALID_ARGUMENT,
	INVALID_STATE
};

class RendererErrorCategory : public std::error_category {
public:
	const char* name() const noexcept override {
		return "Renderer";
	}

	std::string message(int ev) const override {
		switch (static_cast<RCode>(ev)) {
		case RCode::SUCCESS:
			return "Success";
		case RCode::OUT_OF_MEMORY:
			return "Out of memory";
		case RCode::HARDWARE_INCAPABILITY:
			return "Hardware incapability";
		case RCode::SOFTWARE_INCAPABILITY:
			return "Software incapability";
		case RCode::TIMEOUT:
			return "Timeout";
		case RCode::NOT_PERMITTED:
			return "Not permitted";
		case RCode::NOT_SUPPORTED:
			return "Not supported";
		case RCode::INVALID_ARGUMENT:
			return "Invalid argument";
		case RCode::INVALID_STATE:
			return "Invalid state";
		default:
			return "Unknown error";
		}
	}

};

inline const std::error_category& renderer_error_category() {
	static RendererErrorCategory CATEGORY;
	return CATEGORY;
}

_NODISCARD inline std::error_code make_error_code(RCode e) {
	return { static_cast<int>(e), renderer_error_category() };
}

namespace std {

	template <>
	struct is_error_code_enum<RCode> : std::true_type {};

}

