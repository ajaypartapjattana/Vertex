#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "renderer/coreH/RenderTypes.h"

class PassRegistry {
public:
	PassID registerPass(const std::string& name) {
		PassID id = nextID++;
		nameToID[name] = id;
		return id;
	}

	PassID getID(const std::string& name) {
		PassID id = nameToID.at(name);
		return id;
	}

private:
	PassID nextID = 0;
	std::unordered_map<std::string, PassID> nameToID;
};