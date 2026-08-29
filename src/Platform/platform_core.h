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
	int16_t x;
	int16_t y;
	const char* title;
};

struct WindowGeomentry {
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
};

struct DisplayContext_T;
using DisplayContext = DisplayContext_T*;

int requestDisplayContext(DisplayContext* const pContext) noexcept;
void destroyDisplayContext(DisplayContext const _Context) noexcept;

struct DisplayWindow_T;
using DisplayWindow = DisplayWindow_T*;

int createDisplayWindow(const DisplayContext _Context, const WindowCreateInfo* const pCreateInfo, DisplayWindow* const pWindow) noexcept;
void destroyDisplayWindow(DisplayContext const _Context, DisplayWindow const _Window) noexcept;

void raiseDisplayWindow(DisplayContext const _Context, DisplayWindow const _Window) noexcept;
int queryWindowGeometry(DisplayContext const _Context, DisplayWindow const _Window, WindowGeomentry* const pGeomentry) noexcept;
int setWindowGeometry(DisplayContext const _Context, DisplayWindow const _Window, const WindowGeomentry* const pGeometry) noexcept;
void setWindowTitle(DisplayContext const _Context, DisplayWindow const _Window, const char* const _Title) noexcept;

using WindowEventFlags = uint32_t;
enum WindowEventFlagBit : WindowEventFlags {
	WINDOW_EVENT_CLOSE_BIT = 1u << 0,
	WINDOW_EVENT_RESIZE_BIT = 1u << 1,
	WINDOW_EVENT_MOVE_BIT = 1u << 2,
	WINDOW_EVENT_FOCUS_GAINED_BIT = 1u << 3,
	WINDOW_EVENT_FOCUS_LOST_BIT = 1u << 4,
	WINDOW_EVENT_ALL_BIT = ~0u
};

struct EventBufferCreateInfo {
	WindowEventFlags eventMask;
	uint32_t size;
};

struct EventBuffer_T;
using EventBuffer = EventBuffer_T*;

int createEventBuffer(const EventBufferCreateInfo* const pCreateInfo, EventBuffer* const pEventBuffer) noexcept;
void destroyEventBuffer(EventBuffer const _EventBuffer) noexcept;

bool pollWindowEvents(DisplayContext const _Context, EventBuffer const _Buffer) noexcept;
void resolveWindowEvents(EventBuffer const _EventBuffer, DisplayWindow const _Window, WindowEventFlags* const pEventFlags) noexcept;

struct VulkanSurfaceDependencyInfo {
	void* context;
	uintptr_t window;
};

void getVulkanSurfaceDependencyInfo(DisplayContext const _Context, DisplayWindow const _Window, VulkanSurfaceDependencyInfo* const pDependencyInfo) noexcept;
