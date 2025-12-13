#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "Input.h"
#include "Camera.h"

enum class TransformMode{
	NONE,
	TRANSLATE,
	ROTATE,
	SCALE
};

enum class TransformAxis{
	NONE,
	X,
	Y,
	Z,
	XY,
	YZ,
	ZX
};

class TransformController{
public:
	TransformController();
	
	//base transform handler:
	void handletransforms(glm::vec3& objectPosition, glm::vec3& objectRotation, glm::vec3& objectScale);

	//externals/state variable:
	bool inTransformationState;

	Camera* camera = nullptr;
	int screenWidth;
	int screenHeight;

	void setCamera(Camera* cam) { camera = cam; }
	void setScreenDimensions(glm::ivec2 extent) {
		screenWidth = extent.x;
		screenHeight = extent.y;
	};

private:
	TransformMode currentMode;
	TransformAxis activeAxis;

	glm::vec2 prevMousePos;

	//glm::vec3* targetPosition;
	//glm::vec3* targetRotation;
	//glm::vec3* targetScale;

	glm::vec3 objInitialPosition;
	glm::vec3 objInitialRotation;
	glm::vec3 objInitialScale;

	glm::vec3 objCursorPointingVec;
	bool has_objCursorPointingVec;
	float objCursorPointingVecMag;

	glm::vec3 cursorRayInitialIntersection;
	bool has_cursorRayInitialIntersection;

	void applyTranslation(glm::vec3& position, const glm::vec2& mousePos);
	void applyRotation(glm::vec3& rotation, const glm::vec3 position, const glm::vec2& mouseDelta);
	void applyScale(glm::vec3& scale, const glm::vec3 position, const glm::vec2& mousePos);

	bool getAxisFromInput();
	bool getModeFromInput();

	void captureObjState(glm::vec3& objectPosition, glm::vec3& objectRotation, glm::vec3& objectScale);
	void resetObjState(glm::vec3& objectPosition, glm::vec3& objectRotation, glm::vec3& objectScale);
};