#include <stdlib.h>
#include <cstring>
#include <cassert>
#include <new>

#include <vulkan/vulkan.h>

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

xcb_atom_t internAtom(xcb_connection_t* const pConnection, const char* _Name) {
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

		DisplayContext const context = new(std::nothrow) DisplayContext_T;
		
		if (!pContext)
			break;

		context->connection = _connection;
		
		context->wmProtocols = wmProtocols;
		context->wmDeleteWindow = wmDeleteWindow;

		*pContext = context;	

		return 0;

	} while (false);

	if (_connection)
		xcb_disconnect(_connection);

	return -1;
}

int createDisplayWindow(const DisplayContext _Context, const WindowCreateInfo* const pCreateInfo, WindowHandle* const pHandle) noexcept {
	xcb_connection_t* const connection = _Context->connection;

	const xcb_setup_t* setup = xcb_get_setup(connection);
	xcb_screen_iterator_t iterator = xcb_setup_roots_iterator(setup);

	xcb_screen_t* screen = iterator.data;

	xcb_window_t window = xcb_generate_id(connection);

	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[] = { screen->black_pixel, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_STRUCTURE_NOTIFY };

	xcb_void_cookie_t cookie = xcb_create_window_checked(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0, pCreateInfo->width, pCreateInfo->height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);

	if (xcb_generic_error_t* error = xcb_request_check(connection, cookie)) {
		std::free(error);
		return -1;
	}

	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(pCreateInfo->title), pCreateInfo->title);
	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, _Context->wmProtocols, XCB_ATOM_ATOM, 32, 1, &_Context->wmDeleteWindow);

	xcb_map_window(connection, window);
	xcb_flush(connection);

	*pHandle = static_cast<uintptr_t>(window);
	return 0;
}

void getVulkanSurfaceDependencyInfo(DisplayContext const _Context, const WindowHandle _Window, VulkanSurfaceDependencyInfo* const pDependencyInfo) noexcept {
	strcpy(pDependencyInfo->SurfaceType, "xcb");
	pDependencyInfo->context = (void*)_Context->connection;
	pDependencyInfo->window = (uintptr_t)_Window;
}

void purgeDisplayWindow(const DisplayContext _Context, WindowHandle _Window) noexcept {
	xcb_connection_t* const connection = reinterpret_cast<xcb_connection_t*>(_Context);
	
	if (!connection)
		return;
	
	const xcb_window_t window = static_cast<xcb_window_t>(_Window);

	if (!window)
		return;

	xcb_unmap_window(connection, window);

	xcb_destroy_window(connection, window);
	xcb_flush(connection);
}

size_t pollWindowEvent(const DisplayContext _Context, WindowEvent* const pEventBuffer, const size_t _BufferSize) noexcept {
	const DisplayContext_T* const context = reinterpret_cast<const DisplayContext_T*>(_Context);

	const WindowEvent* const pEnd = pEventBuffer + _BufferSize;
	
	for(WindowEvent* pEvent{pEventBuffer}; pEvent != pEnd;) {
		xcb_generic_event_t* const event = xcb_poll_for_event(context->connection);

		if (!event)
			return static_cast<size_t>(pEvent - pEventBuffer);

		const uint8_t type = event->response_type & ~0x80;

		switch (type) {
		case XCB_CONFIGURE_NOTIFY: {
			const xcb_configure_notify_event_t* const configure = reinterpret_cast<xcb_configure_notify_event_t*>(event);

			pEvent->window = static_cast<WindowHandle>(configure->window);
			pEvent->type = WINDOW_EVENT_TYPE_WINDOW_RESIZE;

			pEvent->eventInfo.extent = { configure->width, configure->height };
		}
			++pEvent;
		
			break;

		case XCB_CLIENT_MESSAGE: {
			const xcb_client_message_event_t* const message = reinterpret_cast<xcb_client_message_event_t*>(event);

			if (message->type != context->wmProtocols)
				break;

			if (message->data.data32[0] != context->wmDeleteWindow)
				break;

			pEvent->window = static_cast<WindowHandle>(message->window);
			pEvent->type = WINDOW_EVENT_TYPE_WINDOW_CLOSE;
		}
			++pEvent;

			break;

		}

		std::free(event);
	}	

	return _BufferSize;
}

void purgeDisplayContext(DisplayContext _Context) noexcept {
	DisplayContext_T* const context = reinterpret_cast<DisplayContext_T*>(_Context);

	if (!context)
		return;

	xcb_disconnect(context->connection);

	delete context;
}

int getWindowGeometry(const DisplayContext _Context, const WindowHandle _Window, WindowGeomentry* const pGeomentry) noexcept {
	assert(pGeomentry);

	xcb_connection_t* const connection = reinterpret_cast<xcb_connection_t*>(_Context);

	const xcb_get_geometry_cookie_t cookie = xcb_get_geometry(connection, static_cast<xcb_window_t>(_Window));
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

void setWindowGeometry(const DisplayContext _Context, const WindowHandle _Window, const WindowGeomentry* const pGeometry) noexcept {
	xcb_connection_t* const connection = reinterpret_cast<xcb_connection_t*>(_Context);
	const xcb_window_t window = static_cast<xcb_window_t>(_Window);
	
	uint32_t values[] = { 
		static_cast<uint32_t>(pGeometry->x),
		static_cast<uint32_t>(pGeometry->y),
		static_cast<uint32_t>(pGeometry->width), 
		static_cast<uint32_t>(pGeometry->height) 
	};

	xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
	xcb_flush(connection);
}

void setWindowTitle(const DisplayContext _Context, const WindowHandle _Window, const char* const _Title) noexcept {
	xcb_connection_t* const connection = reinterpret_cast<xcb_connection_t*>(_Context);
	const xcb_window_t window = static_cast<xcb_window_t>(_Window);
	
	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(_Title), _Title);

	xcb_flush(connection);
}

  #elif defined(WINDOW_X11)
    #include <X11/Xlib.h>
  #elif defined(WINDOW_WAYLAND)
    #include <wayland-client.h>
  #else
	#error "Window backend not specified"
  #endif
#endif
