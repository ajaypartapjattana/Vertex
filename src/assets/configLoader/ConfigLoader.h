#pragma once

#include <string>
#include <vector>

#include "XMLLoader/XMLConfigLoader.h"

class ConfigLoader {
public:
	void Build_StartupConfig(const std::string& path);

private:
	const ML_Attribute* FindAttr(const ML_Node& node, const std::string& name);
	const ML_Node* FindChild(const ML_Node& node, const std::string& name);

};