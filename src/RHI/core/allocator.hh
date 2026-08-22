#pragma once

#include <external/VMA/vma.h>
#include <utility>

namespace rhi {

	class allocator {
	private:
		VmaAllocator m_allocator = VK_NULL_HANDLE;

		static const VmaVulkanFunctions m_vkfuncs;

	public:
		allocator() = default;
		allocator(VmaAllocatorCreateInfo* pCreateInfo) {
			create(pCreateInfo);
		}
		allocator(const allocator&) = delete;
		allocator(allocator&& other) noexcept
			:
			m_allocator(std::exchange(other.m_allocator, VK_NULL_HANDLE))
		{

		}

		allocator& operator=(const allocator&) = delete;
		allocator& operator=(allocator&& other) noexcept {
			if (this == &other)
				return *this;

			reset();

			m_allocator = std::exchange(other.m_allocator, VK_NULL_HANDLE);

			return *this;
		}

		~allocator() noexcept {
			reset();
		}

		operator VmaAllocator() const noexcept {
			return m_allocator;
		}

		VkResult create(VmaAllocatorCreateInfo* pCreateInfo) noexcept;
		void reset() noexcept;

	};

}