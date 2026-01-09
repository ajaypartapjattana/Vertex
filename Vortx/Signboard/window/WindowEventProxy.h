#pragma once

class Renderer;
class GLFWwindow;

class WindowEventProxy{
public:
	void attachWindow(GLFWwindow* window);

	void bindRenderer(Renderer* renderer);

	void onFrameBufferResize(int width, int height);
	void onFileDrop(int count, const char** paths);

private:
	GLFWwindow* window = nullptr;
	Renderer* renderer = nullptr;
};