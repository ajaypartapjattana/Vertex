#pragma once

#include <string>

struct WindowInfo {
	int width = 1280;
	int height = 720;
	std::string title = "My Application";
};

struct StartupConfig {
	std::string configName;
	WindowInfo windowCreateInfo;
};