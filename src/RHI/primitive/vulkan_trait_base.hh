#pragma once

#include <vulkan/vulkan.h>

namespace rhi {

	struct vulkan_primitive_base {
		using parent_type = VkDevice;
		using result_type = VkResult;

		static constexpr auto null() noexcept {
			return VK_NULL_HANDLE;
		}

		static constexpr result_type success() noexcept {
			return VK_SUCCESS;
		}

		static constexpr result_type invalid() noexcept {
			return VK_ERROR_INVALID_EXTERNAL_HANDLE;
		}

		static constexpr result_type impermissible() noexcept {
			return VK_ERROR_NOT_PERMITTED;
		}

	};

}