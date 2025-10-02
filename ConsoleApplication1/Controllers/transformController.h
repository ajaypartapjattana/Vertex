#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "Input.h"

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
	Z
};

class TransformController{
public:
	TransformController();
	void update(glm::vec3& objectPosition, glm::vec3& objectRotation, glm::vec3& objectScale, float deltaTime);
	void setMode(TransformMode mode);

private:
	TransformMode currentMode;
	TransformAxis activeAxis;

	glm::vec2 prevMousePos;
	bool dragging;

	TransformAxis getAxisFromInput();
	void applyTranslation(glm::vec3& position, const glm::vec2& mouseDelta);
	void applyRotation(glm::vec3& rotation, const glm::vec2& mouseDelta);
	void applyScale(glm::vec3& scale, const glm::vec2& mouseDelta);
};