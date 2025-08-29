#include "Input.h"

std::unordered_map<int, bool> Input::keys;
std::unordered_map<int, bool> Input::keysPrev;
std::unordered_map<int, bool> Input::mouseButtons;
std::unordered_map<int, bool> Input::mouseButtonsPrev;

glm::vec2 Input::mousePos(0.0f);
glm::vec2 Input::mousePosPrev(0.0f);
glm::vec2 Input::scrollDelta(0.0f);

void Input::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	Input::scrollDelta = glm::vec2(xoffset, yoffset);
}

void Input::update(GLFWwindow* window) {
	keysPrev = keys;
	mouseButtonsPrev = mouseButtons;
	mousePosPrev = mousePos;
	scrollDelta = glm::vec2(0.0f);

	for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
		keys[key] = (glfwGetKey(window, key) == GLFW_PRESS);
	}
	for (int button = GLFW_MOUSE_BUTTON_1; button <= GLFW_MOUSE_BUTTON_LAST; ++button) {
		mouseButtons[button] = (glfwGetMouseButton(window, button) == GLFW_PRESS);
	}

	double x, y;
	glfwGetCursorPos(window, &x, &y);
	mousePos = glm::vec2(x, y);

	glfwSetScrollCallback(window, scrollCallback);
}

bool Input::isKeyPressed(int key) {
	return keys[key] && !keysPrev[key];
}

bool Input::isKeyDown(int key) {
	return keys[key] && keysPrev[key];
}

bool Input::isKeyReleased(int key) {
	return !keys[key] && keysPrev[key];
}

bool Input::isMouseButtonPressed(int button) {
	return mouseButtons[button] && !mouseButtonsPrev[button];
}

bool Input::isMouseButtonDown(int button) {
	return mouseButtons[button] && mouseButtonsPrev[button];
}

bool Input::isMouseButtonReleased(int button) {
	return !mouseButtons[button] && mouseButtonsPrev[button];
}

glm::vec2 Input::getMousePosition() {
	return mousePos;
}

glm::vec2 Input::getMouseDelta() {
	return mousePos - mousePosPrev;
}

glm::vec2 Input::getScrollDelta() {
	return scrollDelta;
}