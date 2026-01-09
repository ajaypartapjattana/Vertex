#pragma once

#include <string>
#include <vector>

struct XMLAttribute {
	std::string name;
	std::string value;
};

struct XMLNode {
	std::string name;
	std::vector<XMLAttribute> attributes;
	std::vector<XMLNode> children;
};