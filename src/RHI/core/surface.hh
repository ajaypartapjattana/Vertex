#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <utility>

namespace rhi {

	class surface {
	private:
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkInstance r_instance = VK_NULL_HANDLE;

	public:
		surface() = default;
		explicit surface(VkInstance instance, GLFWwindow* window) {
			VkResult result = create(instance, window);
			if (result != VK_SUCCESS)
				throw std::runtime_error("FAILURE: surface_creation!");
		}
		surface(const surface&) = delete;
		surface(surface&& other) noexcept
			:
			m_surface(std::exchange(other.m_surface, VK_NULL_HANDLE)),
			r_instance(std::exchange(other.r_instance, VK_NULL_HANDLE))
		{

		}

		surface& operator=(const surface&) = delete;
		surface& operator=(surface&& other) noexcept {
			if (this == &other)
				return *this;

			reset();

			m_surface = std::exchange(other.m_surface, VK_NULL_HANDLE);
			r_instance = std::exchange(other.r_instance, VK_NULL_HANDLE);

			return *this;
		}

		~surface() noexcept {
			reset();
		}

		inline operator VkSurfaceKHR() const noexcept {
			return m_surface;
		}

		VkResult create(VkInstance instance, GLFWwindow* window) noexcept;
		void reset() noexcept;

	};

}