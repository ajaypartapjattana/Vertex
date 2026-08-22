#pragma once

#include "vma_trait_base.h"

namespace rhi {

	struct buffer : vma_primitive_base {
		using handle_type = VkBuffer;
		using createInfo_type = VkBufferCreateInfo;

		static result_type create(parent_type allocator, const createInfo_type* pCreateInfo, const allocationCreateInfo_type* pAllocationCreateInfo, handle_type* pBuffer, allocation_type* pAllocation, allocationInfo_type* pAllocationInfo) noexcept {
			return vmaCreateBuffer(allocator, pCreateInfo, pAllocationCreateInfo, pBuffer, pAllocation, pAllocationInfo);
		}

		static void destroy(parent_type allocator, handle_type buffer, allocation_type allocation) noexcept {
			vmaDestroyBuffer(allocator, buffer, allocation);
		}
	};

}