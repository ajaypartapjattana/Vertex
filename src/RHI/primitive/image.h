#pragma once

#include "vma_trait_base.h"

namespace rhi {

	struct image : vma_primitive_base {
		using handle_type = VkImage;
		using createInfo_type = VkImageCreateInfo;

		static result_type create(parent_type allocator, const createInfo_type* pCreateInfo, const allocationCreateInfo_type* pAllocationCreateInfo, handle_type* pImage, allocation_type* pAllocation, allocationInfo_type* pAllocationInfo) noexcept {
			return vmaCreateImage(allocator, pCreateInfo, pAllocationCreateInfo, pImage, pAllocation, pAllocationInfo);
		}

		static void destroy(parent_type allocator, handle_type image, allocation_type allocation) noexcept {
			vmaDestroyImage(allocator, image, allocation);
		}
	};

}