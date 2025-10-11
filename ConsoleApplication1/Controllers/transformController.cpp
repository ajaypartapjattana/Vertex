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
		has_cursorRayInitialIntersection = false;
		inTransformationState = false;
		return;
	} else if (Input::isKeyPressed(GLFW_KEY_ESCAPE) || Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
		resetObjState(objectPosition, objectRotation, objectScale);
		currentMode = TransformMode::NONE;
		activeAxis = TransformAxis::NONE;
		has_objCursorPointingVec = false;
		has_cursorRayInitialIntersection = false;
		inTransformationState = false;
	}

	if(inTransformationState) {
		if (getAxisFromInput()) {
			resetObjState(objectPosition, objectRotation, objectScale);
			has_cursorRayInitialIntersection = false;
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
			applyScale(objectScale, objectPosition, mousePos);
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
		has_cursorRayInitialIntersection = false;
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
		activeAxis = (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT)) ? TransformAxis::YZ : TransformAxis::X;
		return true;
	}
	if (Input::isKeyPressed(GLFW_KEY_Y)) {
		activeAxis = (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT)) ? TransformAxis::XY : TransformAxis::Z;
		return true;
	}
	if (Input::isKeyPressed(GLFW_KEY_Z)) {
		activeAxis = (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT)) ? TransformAxis::ZX : TransformAxis::Y;
		return true;
	}
	return false;
}

void TransformController::applyTranslation(glm::vec3& position, const glm::vec2& mousePos) {
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix();
	glm::vec3 cameraPos = camera->getPosition();

	glm::vec3 planeNormal;
	switch (activeAxis) {
	case TransformAxis::NONE:
		planeNormal = camera->getForward();
		break;
	case TransformAxis::X:
		planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case TransformAxis::Y:
		planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	case TransformAxis::Z:
		planeNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case TransformAxis::XY:
		planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	case TransformAxis::YZ:
		planeNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case TransformAxis::ZX:
		planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	}

	glm::vec3 rayDir = vecMath::getMouseWorldRay(mousePos, view, projection, screenWidth, screenHeight);

	glm::vec3 objPlaneIntersection;
	if (!vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormal, objPlaneIntersection)) return;
	
	if (!has_cursorRayInitialIntersection) {
		cursorRayInitialIntersection = objPlaneIntersection;
		has_cursorRayInitialIntersection = true;
		return;
	}
	glm::vec3 translation = objPlaneIntersection - cursorRayInitialIntersection;

	switch (activeAxis) {
	case TransformAxis::NONE:
		position = objInitialPosition + translation;
		break;
	case TransformAxis::X:
		position = glm::vec3(objInitialPosition.x + translation.x, objInitialPosition.y, objInitialPosition.z);
		break;
	case TransformAxis::Y:
		position = glm::vec3(objInitialPosition.x, objInitialPosition.y + translation.y, objInitialPosition.z);
		break;
	case TransformAxis::Z:
		position = glm::vec3(objInitialPosition.x, objInitialPosition.y, objInitialPosition.z + translation.z);
		break;
	case TransformAxis::XY:
		position = glm::vec3(objInitialPosition.x + translation.x, objInitialPosition.y + translation.y, objInitialPosition.z);
		break;
	case TransformAxis::YZ:
		position = glm::vec3(objInitialPosition.x, objInitialPosition.y + translation.y, objInitialPosition.z + translation.z);
		break;
	case TransformAxis::ZX:
		position = glm::vec3(objInitialPosition.x + translation.x, objInitialPosition.y, objInitialPosition.z + translation.z);
		break;
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
		planeNormal = camera->getForward();
		rotationAxis = -camera->getForward();
		break;
	case TransformAxis::X:
		planeNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case TransformAxis::Y:
		planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
		rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case TransformAxis::Z:
		planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	default: break;
	}
	glm::vec3 rayDir = vecMath::getMouseWorldRay(mousePos, view, projection, screenWidth, screenHeight);
	glm::vec3 objPlaneIntersection;
	glm::vec3 Norm_CursorPointingVec;

	if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormal, objPlaneIntersection)) {
		Norm_CursorPointingVec = glm::normalize(vecMath::getCursorPointingVec(position, objPlaneIntersection));
		if (!has_objCursorPointingVec) {
			objCursorPointingVec = Norm_CursorPointingVec;
			has_objCursorPointingVec = true;
			return;
		}
		glm::vec3 axis = glm::cross(objCursorPointingVec, Norm_CursorPointingVec);
		float sign = glm::dot(axis, rotationAxis) < 0.0f ? -1.0f : 1.0f;
		float angle = glm::acos(glm::clamp(glm::dot(objCursorPointingVec, Norm_CursorPointingVec), -1.0f, 1.0f));
		rotation = objInitialRotation + glm::degrees(angle) * rotationAxis * sign;
	}
}

void TransformController::applyScale(glm::vec3& scale, const glm::vec3 position, const glm::vec2& mousePos) {
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix();
	glm::vec3 cameraPos = camera->getPosition();
	glm::vec3 scalingPlaneVec = camera->getForward();

	glm::vec3 rayDir = vecMath::getMouseWorldRay(mousePos, view, projection, screenWidth, screenHeight);

	glm::vec3 objPlaneIntersection;
	if (!vecMath::intersectRayPlane(cameraPos, rayDir, position, scalingPlaneVec, objPlaneIntersection)) return;

	glm::vec3 CursorPointingVec = vecMath::getCursorPointingVec(position, objPlaneIntersection);

	if (!has_objCursorPointingVec) {
		objCursorPointingVec = CursorPointingVec;
		objCursorPointingVecMag = glm::length(CursorPointingVec);
		has_objCursorPointingVec = true;
		return;
	}
	float scaling = glm::length(CursorPointingVec) - objCursorPointingVecMag;
	switch (activeAxis) {
	case TransformAxis::NONE:
		scale = objInitialScale + glm::vec3(scaling);
		break;
	case TransformAxis::X:
		scale.x = objInitialScale.x + scaling;
		break;
	case TransformAxis::Y:
		scale.y = objInitialScale.y + scaling;
		break;
	case TransformAxis::Z:
		scale.z = objInitialScale.z + scaling;
		break;
	case TransformAxis::XY:
		scale = glm::vec3(objInitialScale.x + scaling, objInitialScale.y + scaling, objInitialScale.z);
		break;
	case TransformAxis::YZ:
		scale = glm::vec3(objInitialScale.x, objInitialScale.y + scaling, objInitialScale.z + scaling);
		break;
	case TransformAxis::ZX:
		scale = glm::vec3(objInitialScale.x + scaling, objInitialScale.y, objInitialScale.z + scaling);
		break;
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

//- to fix freemode rotation controller.