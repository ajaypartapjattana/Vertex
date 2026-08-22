#include "surface.hh"

namespace rhi {

	VkResult surface::create(VkInstance instance, GLFWwindow* window) noexcept {
		VkSurfaceKHR surface;
		VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
		if (result != VK_SUCCESS)
			return result;

		m_surface = surface;
		r_instance = instance;

		return VK_SUCCESS;
	}

	void surface::reset() noexcept {
		if (m_surface)
			vkDestroySurfaceKHR(r_instance, m_surface, nullptr);

		m_surface = VK_NULL_HANDLE;
	}

}