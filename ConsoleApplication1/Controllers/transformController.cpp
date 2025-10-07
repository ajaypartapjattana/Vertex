#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "transformController.h"
#include "Utilities/vecMath.h"

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
		has_objCursorPointingVec = false;
		has_objCursorRelativeVec = true;
		inTransformationState = false;
		return;
	}

	if (Input::isKeyPressed(GLFW_KEY_ESCAPE) || Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
		resetObjState(objectPosition, objectRotation, objectScale);
		currentMode = TransformMode::NONE;
		activeAxis = TransformAxis::NONE;
		has_objCursorPointingVec = false;
		has_objCursorRelativeVec = false;
		inTransformationState = false;
	}

	if(inTransformationState) {
		if (getAxisFromInput()) {
			resetObjState(objectPosition, objectRotation, objectScale);
			has_objCursorPointingVec = false;
		}

		glm::vec2 mousePos = Input::getMousePosition();
		glm::vec2 mouseDelta = mousePos - prevMousePos;
		prevMousePos = mousePos;

		switch (currentMode) {
		case TransformMode::TRANSLATE:
			applyTranslation(objectPosition, mousePos);
			break;
		case TransformMode::ROTATE:
			applyRotation(objectRotation, objectPosition, mousePos);
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
		has_objCursorRelativeVec = false;
		return newTransform;
	}
	if (Input::isKeyPressed(GLFW_KEY_R)) {
		currentMode = TransformMode::ROTATE;
		has_objCursorPointingVec = false;
		return newTransform;
	}
	if (Input::isKeyPressed(GLFW_KEY_S)) {
		currentMode = TransformMode::SCALE;
		return newTransform;
	}
	return false;
}

bool TransformController::getAxisFromInput() {
	if (Input::isKeyPressed(GLFW_KEY_X)) {
		activeAxis = TransformAxis::X;
		return true;
	}
	if (Input::isKeyPressed(GLFW_KEY_Y)) {
		activeAxis = TransformAxis::Y;
		return true;
	}
	if (Input::isKeyPressed(GLFW_KEY_Z)) {
		activeAxis = TransformAxis::Z;
		return true;
	}
	return false;
}

void TransformController::applyTranslation(glm::vec3& position, const glm::vec2& mousePos) {
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix();
	glm::vec3 cameraPos = camera->getPosition();
	glm::vec3 cameraNplane = camera->getForward();
	glm::vec3 planeNormalY(0.0f, 1.0f, 0.0f);
	glm::vec3 planeNormalZ(0.0f, 0.0f, 1.0f);

	if (!has_objCursorRelativeVec) {
		glm::vec2 objScreenCoord = vecMath::getObjScreenCoord(position, view, projection, screenWidth, screenHeight);
		objCursorRelativeVec = mousePos - objScreenCoord;
		has_objCursorRelativeVec = true;
		return;
	}

	glm::vec3 rayDir = vecMath::getMouseWorldRay(mousePos - objCursorRelativeVec, view, projection, screenWidth, screenHeight);
	glm::vec3 objPlaneIntersection;

	switch(activeAxis) {
	case TransformAxis::NONE : {
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, cameraNplane, objPlaneIntersection)) position = objPlaneIntersection;
		break;
	}
	case TransformAxis::X : {
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormalY, objPlaneIntersection)) position.x = objPlaneIntersection.x;
		break;
	}
	case TransformAxis::Y : {
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormalY, objPlaneIntersection)) position.z = objPlaneIntersection.z;
		break;
	}
	case TransformAxis::Z : {
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormalZ, objPlaneIntersection)) position.y = objPlaneIntersection.y;
		break;
	}
	}
}

void TransformController::applyRotation(glm::vec3& rotation, const glm::vec3 position, const glm::vec2& mousePos) {
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix();
	glm::vec3 cameraPos = camera->getPosition();
	glm::vec3 planeNormal;
	glm::vec3 rotationAxis;

	switch (activeAxis) {
	case TransformAxis::NONE:
		planeNormal = -camera->getForward();
		rotationAxis = camera->getForward();
		break;
	case TransformAxis::X:
		planeNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case TransformAxis::Y:
		planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	case TransformAxis::Z:
		planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
		rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	}
	glm::vec3 rayDir = vecMath::getMouseWorldRay(mousePos, view, projection, screenWidth, screenHeight);
	glm::vec3 objPlaneIntersection;
	glm::vec3 Norm_CursorPointingVec;

	if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormal, objPlaneIntersection)) {
		Norm_CursorPointingVec = glm::normalize(vecMath::getCursorPointingVec(position, objPlaneIntersection));
		if (!has_objCursorPointingVec) {
			objCursorPointingVec = Norm_CursorPointingVec;
			//std::cout << objCursorPointingVec.x << " " << objCursorPointingVec.y << " " << objCursorPointingVec.z << std::endl;
			has_objCursorPointingVec = true;
			return;
		}
		glm::vec3 axis = glm::cross(objCursorPointingVec, Norm_CursorPointingVec);
		float angle = glm::acos(glm::clamp(glm::dot(objCursorPointingVec, Norm_CursorPointingVec), -1.0f, 1.0f));
		float sign = glm::dot(axis, rotationAxis) < 0.0f ? -1.0f : 1.0f;
		rotation = glm::degrees(angle) * rotationAxis * sign;
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