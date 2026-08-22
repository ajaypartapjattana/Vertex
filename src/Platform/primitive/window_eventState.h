#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <utility>

#include "Signboard/Core/Containers/span/span.h"
#include "Signboard/Core/Frame/Frame_event.h"

namespace plf {

	struct glfwCallbacks;

	class windowEventState {
	private:
		friend struct glfwCallbacks;

		std::vector<InputEvent> inputs;
		std::vector<std::string> fileDrops;

		glm::vec2 cursorPosition = { 0.0f, 0.0f };
		glm::vec2 cursorDelta = { 0.0f, 0.0f };
		glm::vec2 scrollDelta = { 0.0f, 0.0f };

		GLFWwindow* r_window;

	public:
		windowEventState() = default;
		windowEventState(GLFWwindow* window) noexcept {
			glfwSetWindowUserPointer(window, this);
			attachWindow(window);
		}
		windowEventState(const windowEventState&) = delete;
		windowEventState(windowEventState&& other) noexcept
			:
			inputs(std::move(other.inputs)),
			fileDrops(std::move(other.fileDrops)),
			r_window(std::exchange(other.r_window, nullptr))
		{
			glfwSetWindowUserPointer(r_window, this);
		}

		windowEventState& operator=(const windowEventState&) = delete;
		windowEventState& operator=(windowEventState&& other) noexcept {
			if (this == &other)
				return *this;

			inputs = std::move(other.inputs);
			fileDrops = std::move(other.fileDrops);
			r_window = std::exchange(other.r_window, nullptr);

			glfwSetWindowUserPointer(r_window, this);

			return *this;
		}

		~windowEventState() noexcept = default;

		void pollEvents() noexcept {
			inputs.clear();
			fileDrops.clear();
			cursorDelta = { 0.0f, 0.0f };
			scrollDelta = { 0.0f, 0.0f };

			glfwPollEvents();
		}

		sgb::span<const InputEvent> input_events() const noexcept {
			return sgb::span<const InputEvent>(inputs.data(), inputs.size());
		}
		sgb::span<const std::string> fileDrop_events() const noexcept {
			return sgb::span<const std::string>(fileDrops.data(), fileDrops.size());
		}
		
		void attachWindow(GLFWwindow* window) noexcept;

	};

}