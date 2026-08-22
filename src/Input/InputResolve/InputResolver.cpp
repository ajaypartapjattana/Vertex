#include "InputResolver.h"

InputResolver::InputResolver() noexcept {

}

void InputResolver::resolveInputs(sgb::span<const InputEvent> events) noexcept {
	m_mousePosPrev = m_mousePos;
	m_scrollDelta = { 0.0f, 0.0f };

	m_keysPrev = m_keys;
	m_mouseButtonsPrev = m_mouseButtons;

	using type = event::InputType;

	for (const InputEvent& e : events) {
		type a_type = event::getType(e.payload);

		switch (a_type) {

		case type::CURSOR_MOVE: {
			auto pos = event::decode_analog(e.payload);
			m_mousePos = glm::vec2{
				(float)pos.x,
				(float)pos.y
			};
			break;
		}

		case type::SCROLL: {
			auto delta = event::decode_analog(e.payload);
			m_scrollDelta += glm::vec2{
				(float)delta.x,
				(float)delta.y
			};
			break;
		}

		case type::KEY_BUTTON: {
			auto key = event::decode_digital(e.payload);
			if (key.flags == GLFW_PRESS)
				m_keys.set(key.payload);
			else if (key.flags == GLFW_RELEASE)
				m_keys.reset(key.payload);
			break;
		}

		case type::MOUSE_BUTTON: {
			auto mouseButton = event::decode_digital(e.payload);
			if (mouseButton.flags == GLFW_PRESS)
				m_mouseButtons.set(mouseButton.payload);
			else if (mouseButton.flags == GLFW_RELEASE)
				m_mouseButtons.reset(mouseButton.payload);
			break;
		}

		default:
			break;
			
		}
	}
}

bool InputResolver::isKeyPressed(int key) const {
	return m_keys.test(key) && !m_keysPrev.test(key);
}

bool InputResolver::isKeyHeld(int key) const {
	return m_keys.test(key);
}

bool InputResolver::isKeyReleased(int key) const {
	return !m_keys.test(key) && m_keysPrev.test(key);
}

bool InputResolver::isMousePressed(int button) const {
	return m_mouseButtons.test(button) && !m_mouseButtonsPrev.test(button);
}

bool InputResolver::isMouseHeld(int button) const {
	return m_mouseButtons.test(button);
}

bool InputResolver::isMouseReleased(int button) const {
	return !m_mouseButtons.test(button) && m_mouseButtonsPrev.test(button);
}

glm::vec2 InputResolver::MousePosition() const {
	return m_mousePos;
}

glm::vec2 InputResolver::MouseDelta() const {
	glm::vec2 delta = m_mousePos - m_mousePosPrev;
	return delta;
}

glm::vec2 InputResolver::ScrollDelta() const {
	glm::vec2 delta = m_scrollDelta;
	return delta;
}