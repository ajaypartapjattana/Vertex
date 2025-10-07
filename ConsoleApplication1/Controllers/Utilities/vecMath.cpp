#include "vecMath.h"

glm::vec3 vecMath::getMouseWorldRay(const glm::vec2& mousePos, const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight) {
	float x = (2.0f * mousePos.x) / screenWidth - 1.0f;
	float y = (2.0f * mousePos.y) / screenHeight - 1.0f;
	glm::vec4 rayClip(x, y, -1.0f, 1.0f);
	glm::vec4 rayEye = glm::inverse(projection) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
	glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
	return rayWorld;
}

bool vecMath::intersectRayPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, glm::vec3& intersectionPoint) {
	float dotPNormal_RDir = glm::dot(planeNormal, rayDir);
	if (fabs(dotPNormal_RDir) < 1e-6f) return false;
	float intersection_RDis = glm::dot(planePoint - rayOrigin, planeNormal) / dotPNormal_RDir;
	if (intersection_RDis < 0) return false;
	intersectionPoint = rayOrigin + intersection_RDis * rayDir;
	return true;
}

glm::vec3 vecMath::getCursorPointingVec(const glm::vec3 objectPosition, const glm::vec3 cursorPlaneIntersection) {
	return cursorPlaneIntersection - objectPosition;
}

glm::vec2 vecMath::getObjScreenCoord(const glm::vec3 objectPosition, glm::mat4 view, glm::mat4 projection, int screenWidth, int screenHeight) {
	glm::vec4 clipSpace = projection * view * glm::vec4(objectPosition, 1.0f);
	glm::vec3 normPos = glm::vec3(clipSpace) / clipSpace.w;
	glm::vec2 screenPos;
	screenPos.x = (normPos.x * 0.5f + 0.5f) * screenWidth;
	screenPos.y = (1.0f - (normPos.y * 0.5f + 0.5f)) * screenHeight;
	return screenPos;
}