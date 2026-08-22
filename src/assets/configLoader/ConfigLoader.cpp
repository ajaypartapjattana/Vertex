#include "ConfigLoader.h"

#include "Signboard/Assets/Utility/convStr.h"

void ConfigLoader::Build_StartupConfig(const std::string& path) {
	XMLConfigLoader XML_config(path);
	ML_Node& root = XML_config.root;

	/*config.configName = FindAttr(root, "name")->value;
	
	const ML_Node* node = FindChild(root, "WindowInfo");
	for (auto& att : node->children) {
		WindowInfo& info = config.windowCreateInfo;
		info.width = convStr::toInt(FindAttr(att, "width")->value.c_str());
		info.height = convStr::toInt(FindAttr(att, "height")->value.c_str());
	}*/

}

const ML_Attribute* ConfigLoader::FindAttr(const ML_Node& node,	const std::string&	name) {
	for (auto& a : node.attributes)
		if (a.name == name)
			return &a;
	return nullptr;
}

const ML_Node* ConfigLoader::FindChild(const ML_Node& node, const std::string&	name) {
	for (auto& c : node.children) {
		if (c.name == name)
			return &c;
	}
	return nullptr;
}