#include "ConfigLoader.h"

ConfigLoader::ConfigLoader(const std::string& path) {
	std::string file_obj = xml.readFile(path);
	data_root = xml.Parse(file_obj);
}

RenderRoutine ConfigLoader::BuildRoutine(const XMLNode& root) {
	RenderRoutine routine;
	routine.name = xml.FindAttr(root, "name")->value;

	const XMLNode* resources = xml.FindChild(root, "Resources");
	for (auto& img : resources->children) {
		PassImage image;
		image.name = img.name;
		image.format = xml.FindAttr(img, "format")->value.c_str();
		image.usage = xml.FindAttr(img, "usage")->value.c_str();
		routine.images.push_back(image);
	}

	return routine;
}