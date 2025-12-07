#include "Camera.h"
#include "Input.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position, float aspectRatio) :
	position(position), worldUp(0.0f, 1.0f, 0.0f), yaw(-135.0f), pitch(-45.0f), fov(45.0f), aspect(1.0f), nearPlane(0.1f), farPlane(1000.0f) {
	updateCameraVectors();
	localMovement = {
		&Camera::moveForwardLocal,
		&Camera::moveBackwardLocal,
		&Camera::moveRightLocal,
		&Camera::moveLeftLocal,
		&Camera::moveUpLocal,
		&Camera::moveDownLocal
	};

	worldMovement = {
		&Camera::moveForwardWorld,
		&Camera::moveBackwardWorld,
		&Camera::moveRightWorld,
		&Camera::moveLeftWorld,
		&Camera::moveUpWorld,
		&Camera::moveDownWorld
	};

	camMode = &localMovement;
}

void Camera::handleCamera(GLFWwindow* window) {
	camMode = (cameraMode == CAMERA_TRANSFORMATION_MODE::LOCAL_MODE) ? &localMovement : &worldMovement;
	float deltaTime = Input::getDeltaTime();

	float moveSpeed = 10.0f * deltaTime;
	float cameraSensitivity = 0.25f * (getCameraFov() / 90.0f);
	glm::vec2 scrollDelta = Input::getScrollDelta();
	if (scrollDelta.y != 0) {
		zoom(scrollDelta.y * 1000.0f * moveSpeed);
	}

	if (Input::isKeyDown(GLFW_KEY_W)) (this->*camMode->forward)(moveSpeed);
	if (Input::isKeyDown(GLFW_KEY_S)) (this->*camMode->backward)(moveSpeed);
	if (Input::isKeyDown(GLFW_KEY_D)) (this->*camMode->right)(moveSpeed);
	if (Input::isKeyDown(GLFW_KEY_A)) (this->*camMode->left)(moveSpeed);
	if (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT)) (this->*camMode->up)(moveSpeed);
	if (Input::isKeyDown(GLFW_KEY_LEFT_CONTROL)) (this->*camMode->down)(moveSpeed);

	if (Input::isKeyDown(GLFW_KEY_RIGHT)) rotate(0.5f, 0);
	if (Input::isKeyDown(GLFW_KEY_LEFT)) rotate(-0.5f, 0);
	if (Input::isKeyDown(GLFW_KEY_UP)) rotate(0, 0.5f);
	if (Input::isKeyDown(GLFW_KEY_DOWN)) rotate(0, -0.5f);

	if (Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glm::vec2 delta = Input::getMouseDelta();
		rotate(delta.x * cameraSensitivity, delta.y * -cameraSensitivity);
	}
	else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Camera::setPosition(const glm::vec3 pos) {
	position = pos;
}

void Camera::makeCameraLookAt(const glm::vec3 pos) {
	position = pos - (front * 5.0f);
}

void Camera::setAspectRatio(float a) {
	if (a <= 0.0f) a = 1.0f;
	aspect = a;
}


//LOCAL
void Camera::moveForwardLocal(float delta) {
	position += front * delta;
}

void Camera::moveBackwardLocal(float delta) {
	position -= front * delta;
}

void Camera::moveRightLocal(float delta) {
	position += right * delta;
}

void Camera::moveLeftLocal(float delta) {
	position -= right * delta;
}

void Camera::moveUpLocal(float delta) {
	position += up * delta;
}

void Camera::moveDownLocal(float delta) {
	position -= up * delta;
}

//WORLD
void Camera::moveForwardWorld(float delta) {
	position += groundForward * delta;
}

void Camera::moveBackwardWorld(float delta) {
	position -= groundForward * delta;
}

void Camera::moveRightWorld(float delta) {
	position += groundRight * delta;
}

void Camera::moveLeftWorld(float delta) {
	position -= groundRight * delta;
}

void Camera::moveUpWorld(float delta) {
	position += worldUp * delta;
}

void Camera::moveDownWorld(float delta) {
	position -= worldUp * delta;
}


void Camera::rotate(float yawDelta, float pitchDelta) {
	yaw += yawDelta;
	pitch += pitchDelta;

	if (pitch > 89.9f) pitch = 89.9f;
	if (pitch < -89.9f) pitch = -89.9f;

	updateCameraVectors();
}

void Camera::zoom(float yOffset) {
	fov -= yOffset;
	if (fov < 1.0f) fov = 1.0f;
	if (fov > 120.0f) fov = 120.0f;
}

glm::mat4 Camera::getViewMatrix() const {
	return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
	glm::mat4 proj =  glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
	proj[1][1] *= -1;
	return proj;
}

void Camera::updateCameraVectors() {
	glm::vec3 f;
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = sin(glm::radians(pitch));
	f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	front = glm::normalize(f);
	groundForward = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
	right = glm::normalize(glm::cross(front, worldUp));
	groundRight = glm::normalize(glm::cross(groundForward, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

float Camera::getCameraFov() {
	return Camera::fov;
}