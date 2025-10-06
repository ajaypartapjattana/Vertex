#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "transformController.h"
#include <iostream>

TransformController::TransformController()
	: currentMode(TransformMode::NONE), activeAxis(TransformAxis::NONE), prevMousePos(0.0f, 0.0f)
{}

void TransformController::handletransforms(glm::vec3& objectPosition, glm::vec3& objectRotation, glm::vec3& objectScale) {
	if (getModeFromInput()) {
		captureObjState(objectPosition, objectRotation, objectScale);
		inTransformationState = true;
		activeAxis = TransformAxis::NONE;
	}

	if (currentMode == TransformMode::NONE) {
		inTransformationState = false;
		activeAxis = TransformAxis::NONE;
		return;
	}

	if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && inTransformationState) {
		currentMode = TransformMode::NONE;
		activeAxis = TransformAxis::NONE;
		inTransformationState = false;
		return;
	}

	if (Input::isKeyPressed(GLFW_KEY_ESCAPE) || Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
		std::cout << "reset state" << std::endl;
		resetObjState(objectPosition, objectRotation, objectScale);
		currentMode = TransformMode::NONE;
		activeAxis = TransformAxis::NONE;
		inTransformationState = false;
	}

	if(inTransformationState) {
		if (getAxisFromInput()) {
			resetObjState(objectPosition, objectRotation, objectScale);
		}

		glm::vec2 mousePos = Input::getMousePosition();
		glm::vec2 mouseDelta = mousePos - prevMousePos;
		prevMousePos = mousePos;

		switch (currentMode) {
		case TransformMode::TRANSLATE:
			applyTranslation(objectPosition, mousePos, mouseDelta);
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
	}
}

bool TransformController::getModeFromInput() {
	bool newTransform = (currentMode == TransformMode::NONE);
	if (Input::isKeyPressed(GLFW_KEY_G)) {
		currentMode = TransformMode::TRANSLATE;
		return newTransform;
	}
	if (Input::isKeyPressed(GLFW_KEY_R)) {
		currentMode = TransformMode::ROTATE;
		return newTransform;
	}
	if (Input::isKeyPressed(GLFW_KEY_S)) {
		currentMode = TransformMode::SCALE;
		return newTransform;
	}
	return 0;
}

bool TransformController::getAxisFromInput() {
	if (Input::isKeyPressed(GLFW_KEY_X)) {
		activeAxis = TransformAxis::X;
		return 1;
	}
	if (Input::isKeyPressed(GLFW_KEY_Y)) {
		activeAxis = TransformAxis::Y;
		return 1;
	}
	if (Input::isKeyPressed(GLFW_KEY_Z)) {
		activeAxis = TransformAxis::Z;
		return 1;
	}
	return 0;
}

void TransformController::applyTranslation(glm::vec3& position, const glm::vec2& mousePos, const glm::vec2 mouseDelta) {
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix();
	glm::vec3 cameraPos = camera->getPosition();
	glm::vec3 cameraNplane = camera->getForward();
	glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
	glm::vec3 rayDir = Input::getMouseWorldRay(mousePos, view, projection, screenWidth, screenHeight);
	glm::vec3 objPlaneIntersection;

	switch(activeAxis) {
	case TransformAxis::NONE : {
		if (Input::intersectRayPlane(cameraPos, rayDir, position, cameraNplane, objPlaneIntersection)) position = objPlaneIntersection;
		break;
	}
	case TransformAxis::X : {
		if (Input::intersectRayPlane(cameraPos, rayDir, position, planeNormal, objPlaneIntersection)) position.x = objPlaneIntersection.x;
		break;
	}
	case TransformAxis::Y: {
		if (Input::intersectRayPlane(cameraPos, rayDir, position, planeNormal, objPlaneIntersection)) position.z = objPlaneIntersection.z;
		break;
	}
	default: break;
	}
	
}

void TransformController::applyRotation(glm::vec3& rotation, const glm::vec2& mouseDelta) {
	float sensitivity = 0.5f;
	switch (activeAxis) {
	case TransformAxis::X: rotation.x += mouseDelta.y * sensitivity; break;
	case TransformAxis::Y: rotation.z += mouseDelta.x * sensitivity; break;
	case TransformAxis::Z: rotation.y += mouseDelta.x * sensitivity; break;
	default: break;
	}
}

void TransformController::applyScale(glm::vec3& scale, const glm::vec2& mouseDelta) {
	float sensitivity = 0.01f;
	switch (activeAxis) {
	case TransformAxis::X: scale.x += mouseDelta.x * sensitivity; break;
	case TransformAxis::Y: scale.z += mouseDelta.y * sensitivity; break;
	case TransformAxis::Z: scale.y += mouseDelta.y * sensitivity; break;
	default: break;
	}
}

void TransformController::captureObjState(glm::vec3& objectPosition, glm::vec3& objectRotation, glm::vec3& objectScale) {
	objInitialPosition = objectPosition;
	objInitialRotation = objectRotation;
	objInitialScale = objectScale;
}

void TransformController::resetObjState(glm::vec3& objectPosition, glm::vec3& objectRotation, glm::vec3& objectScale) {
	objectPosition = objInitialPosition;
	objectRotation = objInitialRotation;
	objectScale = objInitialScale;
}