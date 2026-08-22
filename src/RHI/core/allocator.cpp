#include "allocator.hh"

namespace rhi {

	VkResult allocator::create(VmaAllocatorCreateInfo* pCreateInfo) noexcept {
		VmaAllocator allocator;
		VkResult result = vmaCreateAllocator(pCreateInfo, &allocator);
		if (result != VK_SUCCESS)
			return result;

		reset();

		m_allocator = allocator;
		
		return VK_SUCCESS;
	}

	void allocator::reset() noexcept {
		if (m_allocator)
			vmaDestroyAllocator(m_allocator);

		m_allocator = VK_NULL_HANDLE;
	}

}