#pragma once

#include <external/VMA/vma.h>

namespace rhi {

	struct vma_primitive_base {
		using allocation_type = VmaAllocation;
		using parent_type = VmaAllocator;
		using allocationCreateInfo_type = VmaAllocationCreateInfo;
		using allocationInfo_type = VmaAllocationInfo;
		using result_type = VkResult;

		static constexpr auto null() noexcept {
			return VK_NULL_HANDLE;
		}

		static constexpr parent_type null_root() noexcept {
			return VK_NULL_HANDLE;
		}

		static constexpr result_type success() noexcept {
			return VK_SUCCESS;
		}

		static constexpr result_type impermissible() noexcept {
			return VK_ERROR_NOT_PERMITTED;
		}

		static constexpr result_type invalid() noexcept {
			return VK_ERROR_INVALID_EXTERNAL_HANDLE;
		}
		
	};

}