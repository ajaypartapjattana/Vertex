#include <stdlib.h>
#include <cstring>
#include <cassert>
#include <new>

#include <vulkan/vulkan.h>

#include <core/Memory/memory.h>

#include "platform_core.h"

#if defined(PLATFORM_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}
}

constexpr LPCWSTR className = L"RendererWindows";

int createPlatformWindow(const WindowCreateInfo* const pCreateInfo, WindowHandle* const pHandle) noexcept {
	HINSTANCE instance = GetModuleHandle(nullptr);

	{
		WNDCLASSEXW classDesc{};
		classDesc.cbSize = sizeof(classDesc);
		classDesc.style = CS_HREDRAW | CS_VREDRAW;
		classDesc.lpfnWndProc = WindowProc;
		classDesc.cbClsExtra = 0;
		classDesc.cbWndExtra = 0;
		classDesc.hInstance = instance;
		classDesc.hIcon = nullptr;
		classDesc.hCursor = LoadCursorW(nullptr, IDC_CROSS);
		classDesc.hbrBackground = nullptr;
		classDesc.lpszMenuName = nullptr;
		classDesc.lpszClassName = className;
		classDesc.hIconSm = nullptr;

		if (!RegisterClassExW(&classDesc))
			return -1;
	}

	HWND hwnd = CreateWindowExW(0, className, pCreateInfo->title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, pCreateInfo->width, pCreateInfo->height, nullptr, nullptr, m_instance, nullptr);

	if(!hwnd) {
		UnregisterClassW(className, instance);
		return -1;
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	pHandle->nativeContext = instance;
	phandle->nativeWindow = hwnd;

	return 0;
}

bool Platform::pollEvents() noexcept {
	MSG msg;

	while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT)
			return false;

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return true;
}

#elif defined(PLATFORM_LINUX)

#if defined(WINDOW_XCB)

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <vulkan/vulkan_xcb.h>

static xcb_atom_t internAtom(xcb_connection_t* const pConnection, const char* _Name) {
	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(pConnection, 0, strlen(_Name), _Name);

	xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(pConnection, cookie, nullptr);

	if (!reply)
		return XCB_ATOM_NONE;

	const xcb_atom_t atom = reply->atom;
	std::free(reply);
	
	return atom;
}

struct DisplayContext_T {
	xcb_connection_t* connection;

	xcb_atom_t wmProtocols;
	xcb_atom_t wmDeleteWindow;
	xcb_atom_t netActiveWindow;
};

int requestDisplayContext(DisplayContext* const pContext) noexcept {
	xcb_connection_t* _connection = nullptr;

	do {
		_connection = xcb_connect(nullptr, nullptr);

		if (xcb_connection_has_error(_connection))
			break;

		const xcb_atom_t wmProtocols = internAtom(_connection, "WM_PROTOCOLS");

		if (wmProtocols == XCB_ATOM_NONE)
			break;

		const xcb_atom_t wmDeleteWindow = internAtom(_connection, "WM_DELETE_WINDOW");

		if (wmDeleteWindow == XCB_ATOM_NONE)
			break;

		const xcb_atom_t netActiveWindow = internAtom(_connection, "_NET_ACTIVE_WINDOW");

		if (netActiveWindow == XCB_ATOM_NONE)
			break;

		DisplayContext const context = new(std::nothrow) DisplayContext_T;
		
		if (!pContext)
			break;

		context->netActiveWindow = netActiveWindow;			
		context->wmProtocols = wmProtocols;
		context->wmDeleteWindow = wmDeleteWindow;
			
		context->connection = _connection;

		*pContext = context;	

		return 0;

	} while (false);

	if (_connection)
		xcb_disconnect(_connection);

	return -1;
}

void destroyDisplayContext(DisplayContext _Context) noexcept {
	xcb_disconnect(_Context->connection);

	delete _Context;
}

struct DisplayWindow_T{
	xcb_window_t window;
	const xcb_screen_t* screen;
	uint16_t width;
	uint16_t height;
	int16_t x;
	int16_t y;
};

int createDisplayWindow(const DisplayContext _Context, const WindowCreateInfo* const pCreateInfo, DisplayWindow* const pWindow) noexcept {
	xcb_connection_t* const connection = _Context->connection;

	xcb_window_t _window{};

	do {
		const xcb_setup_t* setup = xcb_get_setup(connection);
		xcb_screen_iterator_t iterator = xcb_setup_roots_iterator(setup);

		xcb_screen_t* screen = iterator.data;

		_window = xcb_generate_id(connection);
		
		uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		uint32_t values[] = { screen->black_pixel, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_FOCUS_CHANGE };
		
		xcb_void_cookie_t cookie = xcb_create_window_checked(connection, XCB_COPY_FROM_PARENT, _window, screen->root, pCreateInfo->x, pCreateInfo->y, pCreateInfo->width, pCreateInfo->height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);

		if (xcb_generic_error_t* error = xcb_request_check(connection, cookie)) {
			std::free(error);
			break;
		}

		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, _window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(pCreateInfo->title), pCreateInfo->title);
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, _window, _Context->wmProtocols, XCB_ATOM_ATOM, 32, 1, &_Context->wmDeleteWindow);

		if (!xcb_flush(connection))
			break;

		DisplayWindow const displayWindow = new(std::nothrow) DisplayWindow_T;

		if (!displayWindow)
			break;

		displayWindow->window = _window;
		displayWindow->screen = screen;
		displayWindow->width = pCreateInfo->width;
		displayWindow->height = pCreateInfo->height;
		displayWindow->x = pCreateInfo->x;
		displayWindow->y = pCreateInfo->y;

		*pWindow = displayWindow;

		return 0;

	} while (false);

	if (_window)
		xcb_destroy_window(connection, _window);

	return -1;
}

void destroyDisplayWindow(const DisplayContext _Context, DisplayWindow const _Window) noexcept {
	xcb_connection_t* const connection = _Context->connection;

	xcb_destroy_window(connection, _Window->window);
	xcb_flush(connection);
}

void raiseDisplayWindow(DisplayContext const _Context, DisplayWindow const _Window) noexcept {
	const xcb_screen_t* const screen = _Window->screen;

	xcb_client_message_event_t event{};
	event.response_type = XCB_CLIENT_MESSAGE;
	event.window = _Window->window;
	event.type = _Context->netActiveWindow;
	event.format = 32;
	event.data.data32[0] = 1;
	event.data.data32[1] = XCB_CURRENT_TIME;
	event.data.data32[2] = XCB_WINDOW_NONE;
	event.data.data32[3] = 0;
	event.data.data32[4] = 0;

	xcb_map_window(_Context->connection, _Window->window);
	xcb_send_event(_Context->connection, 0, screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, reinterpret_cast<const char*>(&event));

	xcb_flush(_Context->connection);
}

int queryWindowGeometry(const DisplayContext _Context, DisplayWindow const _Window, WindowGeomentry* const pGeomentry) noexcept {
	xcb_connection_t* const connection = _Context->connection;

	const xcb_get_geometry_cookie_t cookie = xcb_get_geometry(connection, _Window->window);
	xcb_get_geometry_reply_t* const reply = xcb_get_geometry_reply(connection, cookie, nullptr);

	if (!reply)
		return -1;

	pGeomentry->x = reply->x;
	pGeomentry->y = reply->y;
	pGeomentry->width = reply->width;
	pGeomentry->height = reply->height;

	std::free(reply);

	return 0;
}

int setWindowGeometry(const DisplayContext _Context, DisplayWindow const _Window, const WindowGeomentry* const pGeometry) noexcept {
	uint32_t values[] = { 
		static_cast<uint32_t>(pGeometry->x),
		static_cast<uint32_t>(pGeometry->y),
		static_cast<uint32_t>(pGeometry->width), 
		static_cast<uint32_t>(pGeometry->height) 
	};
	
	xcb_connection_t* const connection = _Context->connection;

	xcb_configure_window(connection, _Window->window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
	
	if (!xcb_flush(connection))
		return -1;

	_Window->width = pGeometry->width;
	_Window->height = pGeometry->height;
	_Window->x = pGeometry->x;
	_Window->y = pGeometry->y;

	return 0;
}

void setWindowTitle(const DisplayContext _Context, DisplayWindow const _Window, const char* const _Title) noexcept {
	xcb_connection_t* const connection = _Context->connection;
	const xcb_window_t window = _Window->window;
	
	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(_Title), _Title);
	xcb_flush(connection);
}

void getVulkanSurfaceDependencyInfo(DisplayContext const _Context, DisplayWindow const _Window, VulkanSurfaceDependencyInfo* const pDependencyInfo) noexcept {
	pDependencyInfo->context = (void*)_Context->connection;
	pDependencyInfo->window = (uintptr_t)_Window->window;
}

enum WindowEventType : uint32_t {
	WINDOW_EVENT_TYPE_WINDOW_CONFIGURE,
	WINDOW_EVENT_TYPE_FOCUS_GAINED,
	WINDOW_EVENT_TYPE_FOCUS_LOST,
	WINDOW_EVENT_TYPE_WINDOW_CLOSE
};

struct WindowEvent {
	xcb_window_t window;
	WindowEventType type;
	struct {
		uint16_t width;
		uint16_t height;
	} extent;
	struct {
		int16_t x;
		int16_t y;
	} position;
};

struct EventBuffer_T {
	WindowEventFlags mask;
	uint32_t count;
	mem::span<WindowEvent> event;
};

int createEventBuffer(const EventBufferCreateInfo* const pCreateInfo, EventBuffer* const pEventBuffer) noexcept {
	mem::span<WindowEvent> _event;

	do {
		_event = { new(std::nothrow) WindowEvent[pCreateInfo->size], (size_t)pCreateInfo->size };

		if (!_event)
			break;

		EventBuffer const buffer = new(std::nothrow) EventBuffer_T;

		if (!buffer)
			break;

		buffer->mask = pCreateInfo->eventMask;
		buffer->count = 0;
		buffer->event = _event;

		*pEventBuffer = buffer;

		return 0;

	} while (false);

	if (_event)
		delete[] _event;

	return -1;
}

void destroyEventBuffer(EventBuffer const _EventBuffer) noexcept {
	delete[] _EventBuffer->event;
	
	delete _EventBuffer;
}

bool pollWindowEvents(DisplayContext const _Context, EventBuffer const _Buffer) noexcept {
	xcb_connection_t* const connection = _Context->connection;

	WindowEvent* pEvent = _Buffer->event.pBegin;
	const WindowEvent* const pEventEnd = _Buffer->event.pEnd;
	while (pEvent != pEventEnd) {
		xcb_generic_event_t* const event = xcb_poll_for_event(connection);

		if (!event)
			break;
		
		const uint8_t type = event->response_type & ~0x80;

		switch (type) {
		case XCB_CONFIGURE_NOTIFY: {
			const xcb_configure_notify_event_t* const configure = reinterpret_cast<xcb_configure_notify_event_t*>(event);

			pEvent->window = configure->window;
			pEvent->type = WINDOW_EVENT_TYPE_WINDOW_CONFIGURE;

			pEvent->extent = { configure->width, configure->height };
			pEvent->position = { configure->x, configure->y };
			
			++pEvent;
		}

			break;

		case XCB_FOCUS_IN: {
			const xcb_focus_in_event_t* const focus = reinterpret_cast<xcb_focus_in_event_t*>(event);

			pEvent->window = focus->event;
			pEvent->type = WINDOW_EVENT_TYPE_FOCUS_GAINED;
			
			++pEvent;
		}

			break;

		case XCB_FOCUS_OUT: {
			const xcb_focus_out_event_t* const focus = reinterpret_cast<xcb_focus_out_event_t*>(event);

			pEvent->window = focus->event;
			pEvent->type = WINDOW_EVENT_TYPE_FOCUS_LOST;
		}

			break;

		case XCB_CLIENT_MESSAGE: {
			const xcb_client_message_event_t* const message = reinterpret_cast<xcb_client_message_event_t*>(event);

			if (message->type != _Context->wmProtocols)
				break;

			if (message->data.data32[0] != _Context->wmDeleteWindow)
				break;

			pEvent->window = message->window;
			pEvent->type = WINDOW_EVENT_TYPE_WINDOW_CLOSE;

			++pEvent;
		}

			break;

		}

		std::free(event);
	}

	_Buffer->count = static_cast<uint32_t>(pEvent - _Buffer->event.pBegin);

	return _Buffer->count != 0;
}

void resolveWindowEvents(EventBuffer const _EventBuffer, DisplayWindow const _Window, WindowEventFlags* const pEventFlags) noexcept {
	const WindowEvent* const pEventEnd = _EventBuffer->event.pBegin + _EventBuffer->count;

	const xcb_window_t window = _Window->window;

	WindowEventFlags events = 0;

	for (const WindowEvent* pEvent{ _EventBuffer->event.pBegin }; pEvent != pEventEnd; ++pEvent) {
		if (pEvent->window != window)
			continue;

		switch (pEvent->type) {
		case WINDOW_EVENT_TYPE_WINDOW_CONFIGURE:
			if (pEvent->extent.width != _Window->width || pEvent->extent.height != _Window->height) {
				events |= WINDOW_EVENT_RESIZE_BIT;
				_Window->width = pEvent->extent.width;
				_Window->height = pEvent->extent.height;
			}
			
			if (pEvent->position.x != _Window->x || pEvent->position.y != _Window->y) {
				events |= WINDOW_EVENT_MOVE_BIT;
				_Window->x = pEvent->position.x;
				_Window->y = pEvent->position.y;
			}
		
			break;
			
		case WINDOW_EVENT_TYPE_FOCUS_GAINED:
			events |= WINDOW_EVENT_FOCUS_GAINED_BIT;

			break;

		case WINDOW_EVENT_TYPE_FOCUS_LOST:
			events |= WINDOW_EVENT_FOCUS_LOST_BIT;

			break;

		case WINDOW_EVENT_TYPE_WINDOW_CLOSE:
			events |= WINDOW_EVENT_CLOSE_BIT;

		default:
			break;
		}
	}

	*pEventFlags |= events & _EventBuffer->mask;
}

  #elif defined(WINDOW_X11)
    #include <X11/Xlib.h>
  #elif defined(WINDOW_WAYLAND)
    #include <wayland-client.h>
  #else
	#error "Window backend not specified"
  #endif
#endif
