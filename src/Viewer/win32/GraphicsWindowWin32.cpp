#include "GraphicsWindowWin32.h"

#include <windowsx.h>
#include <vector>

graphics::GraphicsWindow * graphics::GraphicsWindow::createGraphicsWindow(int x, int y, int width, int height, const char* title)
{
	viewer::GraphicsWindowWin32* ret = new viewer::GraphicsWindowWin32;
	if (ret && ret->init(title, x, y, width, height))
	{
	}
	else
	{
		delete ret;
		ret = 0;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////
//         Window procedure for all GraphicsWindowWin32 instances
//           Dispatches the call to the actual instance
//////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	viewer::GraphicsWindowWin32* window = nullptr;
	return window ? window->handleNativeWindowingEvent(hwnd, uMsg, wParam, lParam) :
		::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

namespace viewer {

GraphicsWindowWin32::GraphicsWindowWin32() :
	GraphicsWindow(),
	_hwnd(0),
	_oldWidth(0),
	_oldHeight(0),
	_frameWidth(0),
	_frameHeight(0),
	_windowClassesRegistered(false),
	_initialized(false),
	_closeWindow(false),
	_frame(true)
{
	SetDllDirectoryA(".");

	registerWindowClasses();
}

GraphicsWindowWin32::~GraphicsWindowWin32()
{
	unregisterWindowClasses();
}

bool GraphicsWindowWin32::init(const char * title, int x, int y, int width, int height)
{
	if (!_windowClassesRegistered) return false;

	if (x < 0)
		x = CW_USEDEFAULT;
	if (y < 0)
		y = CW_USEDEFAULT;

	DisplayDevices displayDevices;
	enumerateDisplayDevices(displayDevices);

	_handle = static_cast<int>(displayDevices.size());


	std::string str(title);

	int destLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), 0, 0);
	if (destLen <= 0)
	{
		LOG_WARN << "Cannot convert UTF-8 string to UTF-16." << std::endl;
	}

	std::wstring sDest(destLen, L'\0');
	destLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &sDest[0], destLen);

	_hwnd = ::CreateWindowEx(
		WS_EX_ACCEPTFILES
		, L"GraphicsWindowWin32"
		, sDest.c_str()
		, WS_OVERLAPPEDWINDOW | WS_VISIBLE // WS_POPUP
		, x
		, y
		, width
		, height
		, NULL
		, NULL
		, GetModuleHandle(NULL)
		, 0
	);

	if (_hwnd == 0)
	{
		LOG_WARN << "GraphicsWindowWin32::init() - Unable to create window" << std::endl;
		return false;
	}

	adjustWindow(width, height, true);
	clearWindow();

	_width = width;
	_height = height;
	_oldWidth = width;
	_oldHeight = height;

	_windowName = title;

	_initialized = true;

	return true;
}

void GraphicsWindowWin32::adjustWindow(int width, int height, bool windowFrame)
{
	_width = width;
	_height = height;
	_aspectRatio = float(width) / float(height);

	::ShowWindow(_hwnd, SW_SHOWNORMAL);

	RECT rect;
	RECT newrect = { 0, 0, (LONG)width, (LONG)height };
	DWORD style = WS_POPUP | WS_SYSMENU;

	if (_frame)
	{
		GetWindowRect(_hwnd, &_rect);
		_style = ::GetWindowLong(_hwnd, GWL_STYLE);
	}

	if (windowFrame)
	{
		rect = _rect;
		style = _style;
	}
	else
	{
		HMONITOR monitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		::GetMonitorInfo(monitor, &mi);
		newrect = mi.rcMonitor;
		rect = mi.rcMonitor;
		_aspectRatio = float(newrect.right - newrect.left) / float(newrect.bottom - newrect.top);
	}

	::SetWindowLong(_hwnd, GWL_STYLE, style);
	uint32_t prewidth = newrect.right - newrect.left;
	uint32_t preheight = newrect.bottom - newrect.top;
	::AdjustWindowRect(&newrect, style, FALSE);
	_frameWidth = (newrect.right - newrect.left) - prewidth;
	_frameHeight = (newrect.bottom - newrect.top) - preheight;
	::UpdateWindow(_hwnd);

	if (rect.left == -32000
		|| rect.top == -32000)
	{
		rect.left = 0;
		rect.top = 0;
	}

	int32_t left = rect.left;
	int32_t top = rect.top;
	int32_t w = (newrect.right - newrect.left);
	int32_t h = (newrect.bottom - newrect.top);

	if (!windowFrame)
	{
		float aspectRatio = 1.0f / _aspectRatio;
		w = MAX(320, w);
		h = uint32_t(float(w)*aspectRatio);

		left = newrect.left + (newrect.right - newrect.left - w) / 2;
		top = newrect.top + (newrect.bottom - newrect.top - h) / 2;
	}

	::SetWindowPos(_hwnd
		, HWND_TOP
		, left
		, top
		, w
		, h
		, SWP_SHOWWINDOW
	);

	::ShowWindow(_hwnd, SW_RESTORE);

	_frame = windowFrame;
}

void GraphicsWindowWin32::clearWindow()
{
	RECT rect;
	::GetWindowRect(_hwnd, &rect);
	HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));
	HDC hdc = GetDC(_hwnd);
	::SelectObject(hdc, brush);
	::FillRect(hdc, &rect, brush);
}

void GraphicsWindowWin32::closeWindow()
{
	destroyWindow();
}

LRESULT GraphicsWindowWin32::handleNativeWindowingEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool GraphicsWindowWin32::checkEvents()
{
	if (!_initialized) return false;

	MSG msg;
	while (::PeekMessage(&msg, _hwnd, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	if (_closeWindow)
	{
		//_closeWindow = false;
		destroyWindow();
	}

	return !_closeWindow;
}

bool GraphicsWindowWin32::registerWindowClasses()
{
	if (_windowClassesRegistered) return true;

	HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);

	WNDCLASSEX wnd;
	memset(&wnd, 0, sizeof(wnd));
	wnd.cbSize = sizeof(wnd);
	wnd.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wnd.lpfnWndProc = WindowProc;
	wnd.hInstance = instance;
	wnd.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wnd.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	//wnd.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1));
	//wnd.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1));
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.lpszClassName = L"GraphicsWindowWin32";

	if (RegisterClassEx(&wnd) == 0)
	{
		unsigned int lastError = ::GetLastError();
		if (lastError != ERROR_CLASS_ALREADY_EXISTS)
		{
			LOG_WARN << "Win32WindowingSystem::registerWindowClasses() - Unable to register first window class" << lastError << std::endl;
			return false;
		}
	}

	_windowClassesRegistered = true;
	return true;
}

void GraphicsWindowWin32::unregisterWindowClasses()
{
	if (_windowClassesRegistered)
	{
		::UnregisterClass(L"GraphicsWindowWin32", ::GetModuleHandle(NULL));

		_windowClassesRegistered = false;
	}
}

void GraphicsWindowWin32::enumerateDisplayDevices(DisplayDevices & displayDevices) const
{
	for (unsigned int deviceNum = 0;; ++deviceNum)
	{
		DISPLAY_DEVICE displayDevice;
		displayDevice.cb = sizeof(displayDevice);

		if (!::EnumDisplayDevices(NULL, deviceNum, &displayDevice, 0)) break;

		// Do not track devices used for remote access (Terminal Services pseudo-displays, etc.)
		if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;

		// Only return display devices that are attached to the desktop
		if (!(displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;

		displayDevices.push_back(displayDevice);
	}
}

void GraphicsWindowWin32::destroyWindow(bool deleteNativeWindow)
{
	if (_hwnd)
	{
		if (deleteNativeWindow)
			::DestroyWindow(_hwnd);
		_hwnd = 0;
	}

	_initialized = false;
	_handle = UINT16_MAX;
}

}