#include "window_eventState.h"

namespace plf {

	static inline windowEventState* from(GLFWwindow* window) {
		return static_cast<windowEventState*>(glfwGetWindowUserPointer(window));
	}

	struct glfwCallbacks {
		static void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods) {
			if (auto* state = from(window)) {
				state->inputs.push_back(InputEvent{ event::encodeKey(key, scanCode, action, mods) });
			}
		}

		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
			if (auto* state = from(window)) {
				state->inputs.push_back(InputEvent{ event::encodeMouse(button, action, mods) });
			}
		}

		static void cursorMoveCallback(GLFWwindow* window, double xPos, double yPos) {
			if (auto* state = from(window)) {

				glm::vec2 newPos{ xPos, yPos };

				state->cursorDelta += newPos - state->cursorPosition;
				state->cursorPosition = newPos;
			}
		}

		static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
			if (auto* state = from(window)) {
				state->scrollDelta += glm::vec2{xOffset, yOffset};
			}
		}

		static void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
			if (auto* state = from(window)) {
				state->fileDrops.reserve(count);
				for (int i = 0; i < count; ++i)
					state->fileDrops.push_back(paths[i]);
			}
		}
	};

	void windowEventState::attachWindow(GLFWwindow* window) noexcept {

		glfwSetDropCallback(window, glfwCallbacks::fileDropCallback);
		glfwSetKeyCallback(window, glfwCallbacks::keyCallback);
		glfwSetScrollCallback(window, glfwCallbacks::scrollCallback);
		glfwSetMouseButtonCallback(window, glfwCallbacks::mouseButtonCallback);
		glfwSetCursorPosCallback(window, glfwCallbacks::cursorMoveCallback);

		r_window = window;
	}

}