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
	} else if (Input::isKeyPressed(GLFW_KEY_ESCAPE) || Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
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
	glm::vec3 cameraNplane = camera->getForward();
	glm::vec3 planeNormalX(1.0f, 0.0f, 0.0f);
	glm::vec3 planeNormalY(0.0f, 1.0f, 0.0f);
	glm::vec3 planeNormalZ(0.0f, 0.0f, 1.0f);

	/*if (!has_objCursorRelativeVec) {
		glm::vec2 objScreenCoord = vecMath::getObjScreenCoord(position, view, projection, screenWidth, screenHeight);
		objCursorRelativeVec = mousePos - objScreenCoord;
		has_objCursorRelativeVec = true;
		return;
	}*/

	glm::vec3 rayDir = vecMath::getMouseWorldRay(mousePos, view, projection, screenWidth, screenHeight);
	glm::vec3 objPlaneIntersection;

	switch(activeAxis) {
	case TransformAxis::NONE:
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, cameraNplane, objPlaneIntersection)) position = objPlaneIntersection;
		break;
	case TransformAxis::X:
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormalY, objPlaneIntersection)) position.x = objPlaneIntersection.x;
		break;
	case TransformAxis::Y:
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormalZ, objPlaneIntersection)) position.y = objPlaneIntersection.y;
		break;
	case TransformAxis::Z:
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormalX, objPlaneIntersection)) position.z = objPlaneIntersection.z;
		break;
	case TransformAxis::XY:
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormalZ, objPlaneIntersection)) position = glm::vec3(objPlaneIntersection.x, objPlaneIntersection.y, position.z);
		break;
	case TransformAxis::YZ:
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormalX, objPlaneIntersection)) position = glm::vec3(position.x, objPlaneIntersection.y, objPlaneIntersection.z);
		break;
	case TransformAxis::ZX:
		if (vecMath::intersectRayPlane(cameraPos, rayDir, position, planeNormalY, objPlaneIntersection)) position = glm::vec3(objPlaneIntersection.x, position.y, objPlaneIntersection.z);
		break;
	default: break;
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
		rotation = glm::degrees(angle) * rotationAxis * sign;
	}
}

void TransformController::applyScale(glm::vec3& scale, const glm::vec3 position, const glm::vec2& mousePos) {
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix();
	glm::vec3 cameraPos = camera->getPosition();
	glm::vec3 scalingPlaneVec;

	switch (activeAxis) {
	case TransformAxis::NONE:
		scalingPlaneVec = camera->getForward();
		break;
	case TransformAxis::X:
		scalingPlaneVec = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case TransformAxis::Y:
		scalingPlaneVec = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case TransformAxis::Z:
		scalingPlaneVec = glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	default: break;
	}
	glm::vec3 rayDir = vecMath::getMouseWorldRay(mousePos, view, projection, screenWidth, screenHeight);
	glm::vec3 objPlaneIntersection;
	glm::vec3 CursorPointingVec;
	if (vecMath::intersectRayPlane(cameraPos, rayDir, position, scalingPlaneVec, objPlaneIntersection)) {
		CursorPointingVec = vecMath::getCursorPointingVec(position, objPlaneIntersection);
		if (!has_objCursorPointingVec) {
			objCursorPointingVec = CursorPointingVec;
			objCursorPointingVecMag = glm::length(CursorPointingVec);
			has_objCursorPointingVec = true;
			return;
		}
		float scaling = glm::length(CursorPointingVec) - objCursorPointingVecMag;
		switch (activeAxis) {
		case TransformAxis::NONE:
			scale += glm::vec3(scaling);
			break;
		case TransformAxis::X:
			scale.x += scaling;
			break;
		case TransformAxis::Y:
			scale.y += scaling;
			break;
		case TransformAxis::Z:
			scale.z += scaling;
			break;
		case TransformAxis::XY:
			scale = glm::vec3(scale.x + scaling, scale.y + scaling, scale.z);
			break;
		case TransformAxis::YZ:
			scale = glm::vec3(scale.x, scale.y + scaling, scale.z + scaling);
			break;
		case TransformAxis::ZX:
			scale = glm::vec3(scale.x + scaling, scale.y, scale.z + scaling);
			break;
		default: break;
		}
		objCursorPointingVecMag = glm::length(CursorPointingVec);
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