#pragma once

#include "InputResolve/InputResolver.h"
#include "Signboard/Core/Interfaces/KeyMappingConfigType.h"

struct FrameCommand;

namespace plf {
	class windowEventState;
}

class CommandDispatcher {
public:
	CommandDispatcher(const InputMapping&& mapping, std::vector<FrameCommand>& appEventQueue, plf::windowEventState& eventState) noexcept;
	
	void resolveInputEvents() noexcept;
	bool ensureWindowVisibility() noexcept;

private:
	bool istriggered(const InputBinding&) const;

private:
	InputMapping m_mapping;
	std::vector<FrameCommand>& targetCommandList;
	
	plf::windowEventState* r_eventState;

	InputResolver m_resolver;

};