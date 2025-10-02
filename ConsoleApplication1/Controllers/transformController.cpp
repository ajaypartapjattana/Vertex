#include "transformController.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

TransformController::TransformController()
	: currentMode(TransformMode::NONE), activeAxis(TransformAxis::NONE), dragging(false), prevMousePos(0.0f, 0.0f)
{}

void TransformController::setMode(TransformMode mode) {
	currentMode = mode;
	//activeAxis = TransformAxis::NONE;
}

void TransformController::update(glm::vec3& objectPosition, glm::vec3& objectRotation, glm::vec3& objectScale, float deltaTime) {
	glm::vec2 mousePos = Input::getMousePosition();
	glm::vec2 mouseDelta = mousePos - prevMousePos;
	prevMousePos = mousePos;

	if (Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
		if (!dragging) {
			dragging = true;
			prevMousePos = Input::getMousePosition();
		}

		activeAxis = getAxisFromInput();
		switch (currentMode) {
		case TransformMode::TRANSLATE:
			applyTranslation(objectPosition, mouseDelta);
			break;
		case TransformMode::ROTATE:
			applyRotation(objectRotation, mouseDelta);
			break;
		case TransformMode::SCALE:
			applyScale(objectScale, mouseDelta);
			break;
		default:
			break;
		}
	} else {
		dragging = false;
		activeAxis = TransformAxis::NONE;
	}
}

TransformAxis TransformController::getAxisFromInput() {
	if (Input::isKeyDown(GLFW_KEY_X)) return TransformAxis::X;
	if (Input::isKeyDown(GLFW_KEY_Y)) return TransformAxis::Y;
	if (Input::isKeyDown(GLFW_KEY_Z)) return TransformAxis::Z;
	return TransformAxis::NONE;
}

void TransformController::applyTranslation(glm::vec3& position, const glm::vec2& mouseDelta) {
	float sensitivity = 0.01f;
	switch (activeAxis) {
	case TransformAxis::X: position.x += mouseDelta.x * sensitivity; break;
	case TransformAxis::Y: position.y += mouseDelta.y * sensitivity; break;
	case TransformAxis::Z: position.z += mouseDelta.y * sensitivity; break;
	default: break;
	}
}

void TransformController::applyRotation(glm::vec3& rotation, const glm::vec2& mouseDelta) {
	float sensitivity = 0.05f;
	switch (activeAxis) {
	case TransformAxis::X: rotation.x += mouseDelta.y * sensitivity; break;
	case TransformAxis::Y: rotation.y += mouseDelta.x * sensitivity; break;
	case TransformAxis::Z: rotation.z += mouseDelta.x * sensitivity; break;
	default: break;
	}
}

void TransformController::applyScale(glm::vec3& scale, const glm::vec2& mouseDelta) {
	float sensitivity = 0.01f;
	switch (activeAxis) {
	case TransformAxis::X: scale.x += mouseDelta.x * sensitivity; break;
	case TransformAxis::Y: scale.y += mouseDelta.y * sensitivity; break;
	case TransformAxis::Z: scale.z += mouseDelta.y * sensitivity; break;
	default: break;
	}
}