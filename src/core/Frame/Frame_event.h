#pragma once

#include <vector>
#include <string>

#include "Signboard/Core/Time/Time.h"
#include "Signboard/Core/Memory/casting.h"

struct InputEvent {
	uint64_t timestamp;
	uint64_t payload;

	InputEvent(uint64_t payload)
		:
		timestamp(Time::now_ns()),
		payload(payload)
	{

	}
};

namespace event {

	enum class InputType : uint8_t {
		KEY_BUTTON,
		MOUSE_BUTTON,
		CURSOR_MOVE,
		SCROLL,
	};

	inline InputType getType(uint64_t e) noexcept {
		return InputType(uint8_t(e >> 56));
	}

	// --- --- ---  --- --- ---  --- --- --- //

	static inline uint64_t encodeKey(int key, int scancode, int action, int mods) noexcept {
		return (uint64_t(InputType::KEY_BUTTON) << 56)
			| (uint64_t(action & 0xFF) << 48)
			| (uint64_t(scancode & 0xFFFF) << 32)
			| (uint64_t(key & 0xFFFFFFFF));
	}

	static inline uint64_t encodeMouse(int button, int action, int mods) noexcept {
		return (uint64_t(InputType::MOUSE_BUTTON) << 56)
			| (uint64_t(action & 0xFF) << 48)
			| (uint64_t(mods & 0xFFFF) << 32)
			| (uint64_t(button & 0xFFFFFFFF));
	}

	/*

		[	8 bits	|	8 bits	|	16 bits	 |	32 bits		]
			TYPE		FLAGS		AUX			PAYLOAD
				<--- INPUT ENCODING LAYOUT --->

	*/

	struct DigitalEvent {
		uint8_t type;
		uint8_t flags;
		uint16_t aux;
		uint32_t payload;
	};

	static inline DigitalEvent decode_digital(uint64_t e) noexcept {
		return {
			uint8_t(e >> 56),
			uint8_t(e >> 48),
			uint16_t(e >> 32),
			uint32_t(e)
		};
	}

	// --- --- ---  --- --- ---  --- --- --- //

	static inline uint64_t encodeCursor(float x, float y) noexcept {
		constexpr float SCALE = 1000.0f;

		int32_t xi = static_cast<int32_t>(x * SCALE);
		int32_t yi = static_cast<int32_t>(y * SCALE);

		uint64_t xbits = uint32_t(xi) & 0x0FFFFFFF;
		uint64_t ybits = uint32_t(yi) & 0x0FFFFFFF;

		return (uint64_t(InputType::CURSOR_MOVE) << 56) | xbits << 28 | ybits;
	}

	static inline uint64_t encodeScroll(float x, float y) noexcept {
		constexpr float SCALE = 1000.0f;

		int32_t xi = static_cast<int32_t>(x * SCALE);
		int32_t yi = static_cast<int32_t>(y * SCALE);

		uint64_t xbits = uint32_t(xi) & 0x0FFFFFFF;
		uint64_t ybits = uint32_t(yi) & 0x0FFFFFFF;

		return (uint64_t(InputType::SCROLL) << 56)
			| (xbits << 28)
			| ybits;
	}

	/*

		[	8 bit	|		28 bits		|		28 bits		]
			TYPE			COMP_X				COMP_Y
				<--- ANALOG ENCODING LAYOUT --->

	*/

	struct AnalogEvent {
		float x;
		float y;
	};

	static inline AnalogEvent decode_analog(uint64_t e) noexcept {
		constexpr float SCALE = 1000.0f;

		uint32_t xbits = (e >> 28) & 0x0FFFFFFF;
		uint32_t ybits = e & 0x0FFFFFFF;

		int32_t xi = (xbits & 0x08000000) ? (xbits | 0xF0000000) : xbits;
		int32_t yi = (ybits & 0x08000000) ? (ybits | 0xF0000000) : ybits;

		return { 
			xi / SCALE,
			yi / SCALE
		};

	}

	// --- --- ---  --- --- ---  --- --- --- //

	static inline uint64_t encodeResize(bool dirty, uint32_t width, uint32_t height) noexcept {
		return (uint64_t(dirty & 0x1) << 63)
			| (uint64_t(width & 0x0FFFFFFF) << 28)
			| (uint64_t(height & 0x0FFFFFFF));
	}

	static inline uint64_t markClean(uint64_t data) noexcept {
		return data & ~(uint64_t(0x1) << 63);
	}

	/*

		[	1 bit	|		7 bits		|		28 bits		|		28 bits		]
			DIRTY			UNUSED				WIDTH				HEIGHT
						<--- RESIZE ENCODING LAYOUT --->

	*/

	struct ResizeData {
		bool dirty;
		uint32_t width;
		uint32_t height;
	};

	static inline ResizeData decode_resize(uint64_t e) noexcept{
		return{
			bool((e >> 63) & 0x1),
			uint32_t(e >> 28) & 0x0FFFFFFF,
			uint32_t(e) & 0x0FFFFFFF
		};
	}


}