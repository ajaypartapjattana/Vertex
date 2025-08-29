#include "Input.h"

std::unordered_map<int, bool> Input::keys;
std::unordered_map<int, bool> Input::keysPrev;
std::unordered_map<int, bool> Input::mouseButtons;
std::unordered_map<int, bool> Input::mouseButtonsPrev;

glm::vec2 Input::mousePos(0.0f);
glm::vec2 Input::mousePosPrev(0.0f);
glm::vec2 Input::scrollDelta(0.0f);

void Input::init(GLFWwindow* window) {
	glfwSetScrollCallback(window, scrollCallBack);
	glfwSetKeyCallback(window, keyCallBack);
	glfwSetCursorPosCallback(window, cursorPosCallBack);
	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
}

void Input::keyCallBack(GLFWwindow* window, int key, int scanCode, int action, int mods) {
	if (action == GLFW_PRESS) keys[key] = true;
	else if (action == GLFW_RELEASE) keys[key] = false;
}

void Input::scrollCallBack(GLFWwindow* window, double xoffset, double yoffset) {
	Input::scrollDelta = glm::vec2(xoffset, yoffset);
}

void Input::mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) mouseButtons[button] = true;
	else if (action == GLFW_RELEASE) mouseButtons[button] = false;
}

void Input::cursorPosCallBack(GLFWwindow* window, double xpos, double ypos) {
	mousePos = glm::vec2(xpos, ypos);

	static bool firstUpdate = true;
	if (firstUpdate) {
		mousePosPrev = mousePos;
		firstUpdate = false;
	}
}

void Input::update(GLFWwindow* window) {
	keysPrev = keys;
	mouseButtonsPrev = mouseButtons;;
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
	glm::vec2 delta = mousePos - mousePosPrev;
	mousePosPrev = mousePos;

	const float MAX_DELTA = 25.0f;
	if (glm::length(delta) > MAX_DELTA) {
		return glm::vec2(0.0f);
	}

	return delta;
}

glm::vec2 Input::getScrollDelta() {
	glm::vec2 delta = scrollDelta;
	scrollDelta = glm::vec2(0.0f);
	return delta;
}