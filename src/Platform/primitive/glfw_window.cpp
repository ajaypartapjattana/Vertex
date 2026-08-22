#include "glfw_window.h"

namespace plf {

	int window::create(uint32_t width, uint32_t height, const char* title) noexcept {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (!window)
			return 0;

		reset();

		m_window = window;

		return 1;
	}

	void window::reset() noexcept {
		if (m_window) {
			glfwDestroyWindow(m_window);
		}

		m_window = nullptr;
	}

}