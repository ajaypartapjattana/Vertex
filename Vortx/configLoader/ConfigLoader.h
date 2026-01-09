#pragma once

#include "XMLLoader/XMLConfigLoader.h"
#include "ReadTypes/EngineReadTypes.h"

class ConfigLoader {
	ConfigLoader(const std::string& path);

	RenderRoutine BuildRoutine(const XMLNode& node);

private:
	XMLConfigLoader xml;

	XMLNode data_root;
};