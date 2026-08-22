#pragma once

#include <string>
#include <vector>

#include "Vertex.h"

struct Model {
	const std::string obj_FilePath;

	struct ModelAttributes {
		size_t trisCount;
		size_t vertCount;
		size_t uniqueVertCount;
	} attributes{ 0 ,0 ,0 };

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Model(const std::string& path)
		:
		obj_FilePath(path)
	{

	}

};