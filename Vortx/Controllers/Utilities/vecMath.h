#pragma once

#include <glm/glm.hpp>

namespace vecMath {
	glm::vec3 getMouseWorldRay(const glm::vec2& mousePos, const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight);
	bool intersectRayPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, glm::vec3& intersectionPoint);
	glm::vec3 getCursorPointingVec(const glm::vec3 objectPosition, const glm::vec3 cursorPlaneIntersection);
	glm::vec2 getObjScreenCoord(const glm::vec3 objectPosition, glm::mat4 view, glm::mat4 projection, int screenWidth, int screenHeight);
}
