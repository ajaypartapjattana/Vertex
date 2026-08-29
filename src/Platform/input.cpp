#include <stdlib.h>
#include <cstring>
#include <cassert>

#include "platform_events.h"

#if defined(PLATFORM_WINDOWS)

#elif defined(PLATFORM_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <libudev.h>

constexpr static InputKey translateLinuxKey(const uint16_t _Symbol) noexcept {
	switch (_Symbol) {
	case KEY_A: return INPUT_KEY_A;
	case KEY_B: return INPUT_KEY_B;
	case KEY_C: return INPUT_KEY_C;
	case KEY_D: return INPUT_KEY_D;
	case KEY_E: return INPUT_KEY_E;
	case KEY_F: return INPUT_KEY_F;
	case KEY_G: return INPUT_KEY_G;
	case KEY_H: return INPUT_KEY_H;
	case KEY_I: return INPUT_KEY_I;
	case KEY_J: return INPUT_KEY_J;
	case KEY_K: return INPUT_KEY_K;
	case KEY_L: return INPUT_KEY_L;
	case KEY_M: return INPUT_KEY_M;
	case KEY_N: return INPUT_KEY_N;
	case KEY_O: return INPUT_KEY_O;
	case KEY_P: return INPUT_KEY_P;
	case KEY_Q: return INPUT_KEY_Q;
	case KEY_R: return INPUT_KEY_R;
	case KEY_S: return INPUT_KEY_S;
	case KEY_T: return INPUT_KEY_T;
	case KEY_U: return INPUT_KEY_U;
	case KEY_V: return INPUT_KEY_V;
	case KEY_W: return INPUT_KEY_W;
	case KEY_X: return INPUT_KEY_X;
	case KEY_Y: return INPUT_KEY_Y;
	case KEY_Z: return INPUT_KEY_Z;
	case KEY_0: return INPUT_KEY_0;
	case KEY_1: return INPUT_KEY_1;
	case KEY_2: return INPUT_KEY_2;
	case KEY_3: return INPUT_KEY_3;
	case KEY_4: return INPUT_KEY_4;
	case KEY_5: return INPUT_KEY_5;
	case KEY_6: return INPUT_KEY_6;
	case KEY_7: return INPUT_KEY_7;
	case KEY_8: return INPUT_KEY_8;
	case KEY_9: return INPUT_KEY_9;
	case KEY_ESC: return INPUT_KEY_ESCAPE;
	case KEY_INSERT: return INPUT_KEY_INSERT;
	case KEY_DELETE: return INPUT_KEY_DELETE;
	case KEY_HOME: return INPUT_KEY_HOME;
	case KEY_END: return INPUT_KEY_END;
	case KEY_PAGEUP: return INPUT_KEY_PAGE_UP;
	case KEY_PAGEDOWN: return INPUT_KEY_PAGE_DOWN;
	case KEY_BACKSPACE: return INPUT_KEY_BACKSPACE;
	case KEY_TAB: return INPUT_KEY_TAB;
	case KEY_ENTER: return INPUT_KEY_ENTER;
	case KEY_SPACE: return INPUT_KEY_SPACE;
	case KEY_LEFT: return INPUT_KEY_ARROW_LEFT;
	case KEY_RIGHT: return INPUT_KEY_ARROW_RIGHT;
	case KEY_UP: return INPUT_KEY_ARROW_UP;
	case KEY_DOWN: return INPUT_KEY_ARROW_DOWN;
	case KEY_LEFTSHIFT: return INPUT_KEY_SHIFT_LEFT;
	case KEY_RIGHTSHIFT: return INPUT_KEY_SHIFT_RIGHT;
	case KEY_LEFTCTRL: return INPUT_KEY_CTRL_LEFT;
	case KEY_RIGHTCTRL: return INPUT_KEY_CTRL_RIGHT;
	case KEY_LEFTALT: return INPUT_KEY_ALT_LEFT;
	case KEY_RIGHTALT: return INPUT_KEY_ALT_RIGHT;
	case KEY_CAPSLOCK: return INPUT_KEY_CAPSLOCK;
	case KEY_NUMLOCK: return INPUT_KEY_NUMLOCK;
	case KEY_SCROLLLOCK: return INPUT_KEY_SCROLL_LOCK;
	case KEY_F1: return INPUT_KEY_F1;
	case KEY_F2: return INPUT_KEY_F2;
	case KEY_F3: return INPUT_KEY_F3;
	case KEY_F4: return INPUT_KEY_F4;
	case KEY_F5: return INPUT_KEY_F5;
	case KEY_F6: return INPUT_KEY_F6;
	case KEY_F7: return INPUT_KEY_F7;
	case KEY_F8: return INPUT_KEY_F8;
	case KEY_F9: return INPUT_KEY_F9;
	case KEY_F10: return INPUT_KEY_F10;
	case KEY_F11: return INPUT_KEY_F11;
	case KEY_F12: return INPUT_KEY_F12;
	case KEY_KP0: return INPUT_KEY_NUMPAD_0;
	case KEY_KP1: return INPUT_KEY_NUMPAD_1;
	case KEY_KP2: return INPUT_KEY_NUMPAD_2;
	case KEY_KP3: return INPUT_KEY_NUMPAD_3;
	case KEY_KP4: return INPUT_KEY_NUMPAD_4;
	case KEY_KP5: return INPUT_KEY_NUMPAD_5;
	case KEY_KP6: return INPUT_KEY_NUMPAD_6;
	case KEY_KP7: return INPUT_KEY_NUMPAD_7;
	case KEY_KP8: return INPUT_KEY_NUMPAD_8;
	case KEY_KP9: return INPUT_KEY_NUMPAD_9;
	case KEY_KPASTERISK: return INPUT_KEY_NUMPAD_MULTIPLY;
	case KEY_KPMINUS: return INPUT_KEY_NUMPAD_SUBTRACT;
	case KEY_KPPLUS: return INPUT_KEY_NUMPAD_ADD;
	case KEY_KPSLASH: return INPUT_KEY_NUMPAD_DIVIDE;
	case KEY_KPENTER: return INPUT_KEY_NUMPAD_ENTER;

	default: return INPUT_KEY_UNDEFINED;
	}
}

template <unsigned short _EventType, unsigned _MaxCodes>
static int hasBits(const int _FileDescriptorIndex, const int* const pCodes, const size_t _CodeCount) noexcept {
	uint8_t bits[(_MaxCodes + 7) / 8]{};

	if (ioctl(_FileDescriptorIndex, EVIOCGBIT(_EventType, sizeof(bits)), bits) < 0)
		return -1;

	const int* const pCodeEnd = pCodes + _CodeCount;
	for (const int* pCode{ pCodes }; pCode != pCodeEnd; ++pCode) {
		if (bits[*pCode / 8] & (1u << (*pCode % 8)))
			continue;

		return -1;
	}

	return 0;
}

static InputDeviceCapabilityFlags getInputDeviceCapability(const int _FileDescriptorIndex, const InputDeviceCapabilityFlags _CapabilityMask) noexcept {
	InputDeviceCapabilityFlags capability = INPUT_DEVICE_CAPABILITY_UNDEFINED_BIT;

	if (_CapabilityMask & INPUT_DEVICE_CAPABILITY_KEYBOARD_BIT) {
		constexpr int keys[7] = { KEY_ESC, KEY_A, KEY_Z, KEY_ENTER, KEY_SPACE, KEY_LEFTSHIFT, KEY_LEFTCTRL };

		capability |= (hasBits<EV_KEY, KEY_MAX>(_FileDescriptorIndex, keys, 7) == 0) ? INPUT_DEVICE_CAPABILITY_KEYBOARD_BIT : 0u;
	}

	if (_CapabilityMask & INPUT_DEVICE_CAPABILITY_MOUSE_BIT) {
		constexpr int axes[2] = { REL_X, REL_Y };
		constexpr int buttons[2] = { BTN_LEFT, BTN_RIGHT };

		capability |= (hasBits<EV_REL, REL_MAX>(_FileDescriptorIndex, axes, 2) == 0 && hasBits<EV_KEY, KEY_MAX>(_FileDescriptorIndex, buttons, 2) == 0) ? INPUT_DEVICE_CAPABILITY_MOUSE_BIT : 0u;
	}
	
	if (_CapabilityMask & INPUT_DEVICE_CAPABILITY_GAMEPAD_BIT) {
		constexpr int buttons[] = { BTN_SOUTH, BTN_EAST, BTN_START };
		constexpr int axes[] = { ABS_X, ABS_Y };

		capability |= (hasBits<EV_ABS, ABS_MAX>(_FileDescriptorIndex, axes, 2) == 0 && hasBits<EV_KEY, KEY_MAX>(_FileDescriptorIndex, buttons, 3) == 0) ? INPUT_DEVICE_CAPABILITY_GAMEPAD_BIT : 0u;
	}

	return capability;    
}

struct LinuxInputDeviceInfo {
	InputDeviceCapabilityFlags capability;
	int fileDescriptorIndex;
	const char* name;
};

struct LinuxInputSet {
	LinuxInputDeviceInfo keyboard;
	LinuxInputDeviceInfo mouse;
	LinuxInputDeviceInfo gamepad;
};

int createInputDeviceSet(InputDeviceSet const pDeviceSet, const InputDeviceCapabilityFlags _InputCapability) noexcept {
	assert(pDeviceSet);
	
	LinuxInputSet* const pSet = reinterpret_cast<LinuxInputSet*>(pDeviceSet);

	udev* const udev = udev_new();

	if (!udev)
		return -1;

	udev_enumerate* const enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "input");

	udev_enumerate_scan_devices(enumerate);

	udev_list_entry* const devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry* entry;

	InputDeviceCapabilityFlags capabilityMask = _InputCapability;

	udev_list_entry_foreach(entry, devices) {
		if (!capabilityMask)
			break;

		const char* sysPath = udev_list_entry_get_name(entry);
		udev_device* device = udev_device_new_from_syspath(udev, sysPath);

		const char* devnode = udev_device_get_devnode(device);
		if(!devnode)
			continue;
		
		if (strncmp(devnode, "/dev/input/event", 16u) != 0)
			continue;

		int fd = open(devnode, O_RDONLY | O_NONBLOCK);

		InputDeviceCapabilityFlags capability = getInputDeviceCapability(fd, capabilityMask);

		if (!capability)
			close(fd);

		if (capability & INPUT_DEVICE_CAPABILITY_KEYBOARD_BIT) {
			pSet->keyboard.capability = capability;
			pSet->keyboard.fileDescriptorIndex = fd;
			pSet->keyboard.name = udev_device_get_property_value(device, "NAME");

			capabilityMask &= ~INPUT_DEVICE_CAPABILITY_KEYBOARD_BIT;
		}

		if (capability & INPUT_DEVICE_CAPABILITY_MOUSE_BIT) {
			pSet->mouse.capability = capability;
			pSet->mouse.fileDescriptorIndex = fd;
			pSet->mouse.name = udev_device_get_property_value(device, "NAME");

			capability &= ~INPUT_DEVICE_CAPABILITY_MOUSE_BIT;
		}

		if (capability & INPUT_DEVICE_CAPABILITY_MOUSE_BIT) {
			pSet->gamepad.capability = capability;
			pSet->gamepad.fileDescriptorIndex = fd;
			pSet->gamepad.name = udev_device_get_property_value(device, "NAME");

			capability &= ~INPUT_DEVICE_CAPABILITY_GAMEPAD_BIT;
		}
	}

	return 0;
}

int readInputFile();

#endif