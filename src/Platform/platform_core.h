#pragma once

#include <cstdint>

typedef uint32_t Flags;

using WindowCreateFlags = Flags;
enum WindowCreateFlagBit : WindowCreateFlags {
	WINDOW_CREATE_RESIZABLE_BIT = 1u << 0,
	WINDOW_CREATE_FULLSCREEN_BIT = 1u << 0
};

struct WindowCreateInfo {
	WindowCreateFlags flags;
	uint16_t width;
	uint16_t height;
	const char* title;
};

struct WindowGeomentry {
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
};

enum WindowEventType : uint32_t {
	WINDOW_EVENT_TYPE_WINDOW_CLOSE,
	WINDOW_EVENT_TYPE_WINDOW_RESIZE,
	WINDOW_EVENT_TYPE_WINDOW_MOVE,
	WINDOW_EVENT_TYPE_FOCUS_GAINED,
	WINDOW_EVENT_TYPE_FOCUS_LOST
};

struct EventExtentInfo {
	uint16_t width;
	uint16_t height;
};

struct EventPositionInfo {
	int16_t x;
	int16_t y;
};

struct EventDeltaInfo {
	int16_t dx;
	int16_t dy;
};

struct DisplayContext_T;
using DisplayContext = DisplayContext_T*;

using WindowHandle = uintptr_t;

struct WindowEvent {
	WindowHandle window;
	WindowEventType type;
	union {
		EventExtentInfo extent;
		EventPositionInfo position;
		EventDeltaInfo delta;
	} eventInfo;
};

int requestDisplayContext(DisplayContext* const pContext) noexcept;
int createDisplayWindow(const DisplayContext _Context, const WindowCreateInfo* const pCreateInfo, WindowHandle* const pHandle) noexcept;

size_t pollWindowEvent(DisplayContext const _Context, WindowEvent* const pEventBuffer, const size_t _BufferSize) noexcept;

void purgeDisplayWindow(DisplayContext const _Context, WindowHandle _Window) noexcept;
void purgeDisplayContext(DisplayContext const _Context) noexcept;

int getWindowGeometry(DisplayContext const _Context, const WindowHandle _Window, WindowGeomentry* const pGeomentry) noexcept;

void setWindowGeometry(DisplayContext const _Context, const WindowHandle _Window, const WindowGeomentry* const pGeometry) noexcept;
void setWindowTitle(DisplayContext const _Context, const WindowHandle _Window, const char* const _Title) noexcept;

struct VulkanSurfaceDependencyInfo {
	char SurfaceType[8];
	void* context;
	uintptr_t window;
};

void getVulkanSurfaceDependencyInfo(DisplayContext const _Context, const WindowHandle _Window, VulkanSurfaceDependencyInfo* const pDependencyInfo) noexcept;
