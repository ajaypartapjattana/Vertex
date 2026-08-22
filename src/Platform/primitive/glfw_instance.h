#pragma once

#include <cstdint>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace plf {

	class instance {
	public:
		instance() {
			if (!glfwInit())
				throw std::runtime_error("FAILURE: glfw_unintialized!");
		}
		instance(const instance&) = delete;
		instance(instance&&) = delete;

		instance& operator=(const instance&) = delete;
		instance& operator=(instance&&) = delete;

		~instance() noexcept {
			glfwTerminate();
		}

	};

}