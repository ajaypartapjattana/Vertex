#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <utility>
#include <stdexcept>

namespace rhi {

	class instance {
	private:
		VkInstance m_instance = VK_NULL_HANDLE;

	public:
		instance() = default;
		instance(const VkInstanceCreateInfo* pCreateInfo) {
			VkResult result = create(pCreateInfo);
			if (result != VK_SUCCESS)
				throw std::runtime_error("FAILURE: vulkan_instance_creation!");
		}
		instance(const instance&) = delete;
		instance(instance&& other) noexcept 
			:
			m_instance(std::exchange(other.m_instance, VK_NULL_HANDLE))
		{

		}
		
		instance& operator=(const instance&) = delete;
		instance& operator=(instance&& other) noexcept {
			if (this == &other)
				return *this;

			reset();

			m_instance = std::exchange(other.m_instance, VK_NULL_HANDLE);

			return *this;
		}

		~instance() noexcept {
			reset();
		}

		inline operator VkInstance() const noexcept {
			return m_instance;
		}

		VkResult create(const VkInstanceCreateInfo* pCreateInfo) noexcept;
		void reset() noexcept;

	};

}
