#pragma once

#include "Signboard/Core/Frame/Frame_command.h"

#include <vector>

enum class InputTrigger {
	Pressed,
	Released,
	Held
};

struct InputBinding {
	int key;
	InputTrigger trigger;
	CommandID command;
};

using InputMapping = std::vector<InputBinding>;