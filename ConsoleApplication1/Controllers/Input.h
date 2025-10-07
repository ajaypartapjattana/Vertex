#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <chrono>

class Input
{
public:
	//synchronizers:
	static void init(GLFWwindow* window);
	static void update(GLFWwindow* window);

	//raw input functions:
	static bool isKeyPressed(int key);
	static bool isKeyDown(int key);
	static bool isKeyReleased(int key);

	static bool isMouseButtonPressed(int button);
	static bool isMouseButtonDown(int button);
	static bool isMouseButtonReleased(int button);

	static glm::vec2 getMousePosition();
	static glm::vec2 getMouseDelta();
	static glm::vec2 getScrollDelta();

	//system event latchers:
	static void keyCallBack(GLFWwindow* window, int key, int scanCode, int action, int mods);
	static void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset);
	static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos);

	//synchronoization Util-func
	static float getDeltaTime();

private:
	static std::unordered_map<int, bool> keys;
	static std::unordered_map<int, bool> keysPrev;
	static std::unordered_map<int, bool> mouseButtons;
	static std::unordered_map<int, bool> mouseButtonsPrev;

	static glm::vec2 mousePos;
	static glm::vec2 mousePosPrev;
	static glm::vec2 scrollDelta;
};