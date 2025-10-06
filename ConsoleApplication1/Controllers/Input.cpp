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

glm::vec3 Input::getMouseWorldRay(const glm::vec2& mousePos, const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight) {
	float x = (2.0f * mousePos.x) / screenWidth - 1.0f;
	float y = (2.0f * mousePos.y) / screenHeight - 1.0f;
	glm::vec4 rayClip(x, y, -1.0f, 1.0f);
	glm::vec4 rayEye = glm::inverse(projection) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
	glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
	return rayWorld;
}

bool Input::intersectRayPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, glm::vec3& intersectionPoint) {
	float dotPNormal_RDir = glm::dot(planeNormal, rayDir);
	if (fabs(dotPNormal_RDir) < 1e-6f) return false;
	float intersection_RDis = glm::dot(planePoint - rayOrigin, planeNormal) / dotPNormal_RDir;
	if (intersection_RDis < 0) return false;
	intersectionPoint = rayOrigin + intersection_RDis * rayDir;
	return true;
}

float Input::getDeltaTime() {
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = currentTime - lastTime;
	lastTime = currentTime;
	return deltaTime.count();
}