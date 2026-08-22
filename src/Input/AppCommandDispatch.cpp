#include "AppCommandDisptach.h"

#include "Signboard/Core/core.h"
#include "Signboard/Platform/platform.h"

CommandDispatcher::CommandDispatcher(const InputMapping&& mapping, std::vector<FrameCommand>& appEventQueue, plf::windowEventState& eventState) noexcept
	:
	m_mapping(mapping),
	targetCommandList(appEventQueue),
	r_eventState(&eventState)
{

}

bool CommandDispatcher::ensureWindowVisibility() noexcept {
	uint64_t _rszState = r_eventState->windowResize_event();

	auto data = event::decode_resize(_rszState);

	bool visibility = data.width != 0 && data.height != 0;

	if (data.dirty) {
		targetCommandList.push_back(makeCommand(CommandID::CONFIG));
		r_eventState->windowResize_markClean();
	}

	return visibility;
}

void CommandDispatcher::resolveInputEvents() noexcept {
	if (!r_eventState)
		return;

	m_resolver.resolveInputs(r_eventState->input_events());

	for (const InputBinding& binding : m_mapping) {
		if(istriggered(binding)) {
			targetCommandList.push_back(makeCommand(binding.command));
		}
	}
}

bool CommandDispatcher::istriggered(const InputBinding& binding) const {
	switch (binding.trigger) {
	case InputTrigger::Pressed:		
		return m_resolver.isKeyPressed(binding.key);

	case InputTrigger::Released:	
		return m_resolver.isKeyReleased(binding.key);
	
	case InputTrigger::Held:		
		return m_resolver.isKeyHeld(binding.key);

	default:
		return false;
	}
}