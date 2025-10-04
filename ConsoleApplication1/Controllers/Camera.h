#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Input.h"

class Camera
{
public:
	Camera(glm::vec3 position, float aspectRatio);

	//camera handler:
	void handleCamera(GLFWwindow* window);

	//camera helpers:
	void setAspectRatio(float aspect);

	void moveForward(float delta);
	void moveBackward(float delta);
	void moveRight(float delta);
	void moveLeft(float delta);
	void moveUp(float delta);
	void moveDown(float delta);

	void rotate(float yawDelta, float pitchDelta);
	void zoom(float yOffset);

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	glm::vec3 getPosition() const { return position; }

	float getCameraFov();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float yaw;
	float pitch;
	float fov;
	float aspect;
	float nearPlane;
	float farPlane;

	void updateCameraVectors();
};