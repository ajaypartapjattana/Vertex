#pragma once

#include <glm/glm.hpp>

struct Global_UBO
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 cameraPos;
	float time;
};