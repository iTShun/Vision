#include "LSWindow-win32.h"

#if PLATFORM_WINDOWS

#include "Utility/LSDynLib.h"
#include "Error/LSException.h"
#include "Logger/LSLogger.h"

// dbt.h is lowercase here, as a cross-compile on linux with mingw-w64
// expects lowercase, and a native compile on windows, whether via msvc
// or mingw-w64 addresses files in a case insensitive manner.
#include <dbt.h>

// MinGW lacks the definition of some Win32 constants
#ifndef XBUTTON1
#	define XBUTTON1 0x0001
#endif
#ifndef XBUTTON2
#	define XBUTTON2 0x0002
#endif
#ifndef WM_MOUSEHWHEEL
#	define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef MAPVK_VK_TO_VSC
#	define MAPVK_VK_TO_VSC (0)
#endif

namespace ls
{
	static UINT32				sWindowCount = 0; // Windows owned
	static UINT32				sHandleCount = 0; // All window handles
	static const wchar_t*		sClassName = L"Win32_Window";
	static UPtr<Win32Window>	sFullscreenWindow(nullptr, nullptr);

	const GUID GUID_DEVINTERFACE_HID = { 0x4d1e55b2, 0xf16f, 0x11cf,{ 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

	void setProcessDpiAware();

	Win32Window::Win32Window()
		: Window()
		, mHandle(NULL)
		, mCallback(0)
		, mFullscreen(false)
		, mResizing(true)
	{
		
	}

	Win32Window::Win32Window(VideoMode mode, const WString& title, WindowStyleFlags style)
		: Window()
		, mHandle(NULL)
		, mCallback(0)
		, mFullscreen(false)
		, mResizing(true)
	{
		create(mode, title, style);
	}

	Win32Window::Win32Window(WindowHandle handle)
		: Window()
		, mHandle(NULL)
		, mCallback(0)
		, mFullscreen(false)
		, mResizing(true)
	{
		create(handle);
	}

	Win32Window::~Win32Window()
	{
		
	}

	void Win32Window::create(VideoMode mode, const WString & title, WindowStyleFlags style)
	{
		Window::create(mode, title, style);

		// Set that this process is DPI aware and can handle DPI scaling
		setProcessDpiAware();

		// Register the window class at first call
		if (sWindowCount == 0)
			registerWindowClass();

		// Compute position and size
		HDC screenDC = GetDC(NULL);
		int left = (GetDeviceCaps(screenDC, HORZRES) - static_cast<int>(mode.width)) / 2;
		int top = (GetDeviceCaps(screenDC, VERTRES) - static_cast<int>(mode.height)) / 2;
		int width = mode.width;
		int height = mode.height;
		ReleaseDC(NULL, screenDC);

		mFullscreen = style & WindowStyle::Fullscreen;

		// Choose the window style according to the Style parameter
		DWORD win32Style = WS_VISIBLE;
		if (style == WindowStyle::None)
		{
			win32Style |= WS_POPUP;
		}
		else
		{
			if (style & WindowStyle::Titlebar) win32Style |= WS_CAPTION | WS_MINIMIZEBOX;
			if (style & WindowStyle::Resize)   win32Style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
			if (style & WindowStyle::Close)    win32Style |= WS_SYSMENU;
		}

		// In windowed mode, adjust width and height so that window will have the requested client area
		if (!mFullscreen)
		{
			RECT rectangle = { 0, 0, width, height };
			AdjustWindowRect(&rectangle, win32Style, false);
			width = rectangle.right - rectangle.left;
			height = rectangle.bottom - rectangle.top;
		}

		// Create the window
		mHandle = CreateWindowW(sClassName, title.c_str(), win32Style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);

		// Register to receive device interface change notifications (used for joystick connection handling)
		DEV_BROADCAST_DEVICEINTERFACE deviceInterface = { sizeof(DEV_BROADCAST_DEVICEINTERFACE), DBT_DEVTYP_DEVICEINTERFACE, 0, GUID_DEVINTERFACE_HID, 0 };
		RegisterDeviceNotification(mHandle, &deviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

		// If we're the first window handle, we only need to poll for joysticks when WM_DEVICECHANGE message is received
		if (mHandle)
		{
			/*if (handleCount == 0)
				JoystickImpl::setLazyUpdates(true);

			++handleCount;Todo*/
		}

		// By default, the OS limits the size of the window the the desktop size,
		// we have to resize it after creation to apply the real size
		setSize(Vector2I(mode.width, mode.height));

		// Switch to fullscreen if requested
		if (mFullscreen)
			switchToFullscreen(mode);

		// Increment window count
		++sWindowCount;
	}

	void Win32Window::create(WindowHandle handle)
	{
		Window::create(handle);

		// Set that this process is DPI aware and can handle DPI scaling
		setProcessDpiAware();

		if (mHandle)
		{
			// If we're the first window handle, we only need to poll for joysticks when WM_DEVICECHANGE message is received
			/*if (handleCount == 0)
				JoystickImpl::setLazyUpdates(true);

			++handleCount;Todo*/

			// We change the event procedure of the control (it is important to save the old one)
			SetWindowLongPtrW(mHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			mCallback = SetWindowLongPtrW(mHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Win32Window::globalOnEvent));
		}
	}

	void Win32Window::close()
	{
		// TODO should we restore the cursor shape and visibility?

		// Destroy the custom icon, if any

		// If it's the last window handle we have to poll for joysticks again

		if (!mCallback)
		{
			// Destroy the window
			if (mHandle)
				DestroyWindow(mHandle);

			// Decrement the window count
			if (sWindowCount != 0)
				--sWindowCount;

			// Unregister window class if we were the last window
			if (sWindowCount == 0)
				UnregisterClassW(sClassName, GetModuleHandleW(NULL));
		}
		else
		{
			// The window is external: remove the hook on its message callback
			SetWindowLongPtrW(mHandle, GWLP_WNDPROC, mCallback);
		}

		// Update the fullscreen window
		if (this == sFullscreenWindow.get())
			sFullscreenWindow = nullptr;
	}

	void Win32Window::setTitle(const WString & title)
	{
		SetWindowTextW(mHandle, title.c_str());
	}

	void Win32Window::setPosition(const Vector2I & position)
	{
		SetWindowPos(mHandle, NULL, position.x, position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		/*if (mCursorGrabbed)
			grabCursor(true);Todo*/
	}

	Vector2I Win32Window::getPosition() const
	{
		RECT rect;
		GetWindowRect(mHandle, &rect);

		return Vector2I(rect.left, rect.top);
	}

	void Win32Window::setSize(const Vector2I & size)
	{
		// SetWindowPos wants the total size of the window (including title bar and borders),
		// so we have to compute it
		RECT rectangle = { 0, 0, static_cast<long>(size.x), static_cast<long>(size.y) };
		AdjustWindowRect(&rectangle, GetWindowLong(mHandle, GWL_STYLE), false);
		int width = rectangle.right - rectangle.left;
		int height = rectangle.bottom - rectangle.top;

		SetWindowPos(mHandle, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
	}

	Vector2I Win32Window::getSize() const
	{
		RECT rect;
		GetClientRect(mHandle, &rect);

		return Vector2I(rect.right - rect.left, rect.bottom - rect.top);
	}

	void Win32Window::setVisible(bool visible)
	{
		ShowWindow(mHandle, visible ? SW_SHOW : SW_HIDE);
	}

	bool Win32Window::isVisible()
	{
		return (mHandle && !IsIconic(mHandle));
	}

	void Win32Window::processEvents()
	{
		// We process the window events only if we own it
		if (!mCallback)
		{
			MSG message;
			while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessageW(&message);
			}
		}
	}

	void Win32Window::registerWindowClass()
	{
		WNDCLASSW windowClass;
		windowClass.style = 0;
		windowClass.lpfnWndProc = &Win32Window::globalOnEvent;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = GetModuleHandleW(NULL);
		windowClass.hIcon = NULL;
		windowClass.hCursor = 0;
		windowClass.hbrBackground = 0;
		windowClass.lpszMenuName = NULL;
		windowClass.lpszClassName = sClassName;
		RegisterClassW(&windowClass);
	}

	void Win32Window::switchToFullscreen(const VideoMode & mode)
	{
		DEVMODEW devMode;
		devMode.dmSize = sizeof(devMode);
		devMode.dmPelsWidth = mode.width;
		devMode.dmPelsHeight = mode.height;
		devMode.dmBitsPerPel = mode.bitsPerPixel;
		devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

		// Apply fullscreen mode
		if (ChangeDisplaySettingsW(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			LOGERR("Failed to change display mode for fullscreen");
			return;
		}

		// Make the window flags compatible with fullscreen mode
		SetWindowLongW(mHandle, GWL_STYLE, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		SetWindowLongW(mHandle, GWL_EXSTYLE, WS_EX_APPWINDOW);

		// Resize the window so that it fits the entire screen
		SetWindowPos(mHandle, HWND_TOP, 0, 0, mode.width, mode.height, SWP_FRAMECHANGED);
		ShowWindow(mHandle, SW_SHOW);

		// Set "this" as the current fullscreen window
		sFullscreenWindow = ls_unique_ptr(this);
	}

	void Win32Window::processEvent(UINT message, WPARAM wParam, LPARAM lParam)
	{
		// Don't process any message until window is created
		if (mHandle == NULL)
			return;

		switch (message)
		{
			// Destroy event
			case WM_DESTROY:
				{
					// Here we must cleanup resources !
					cleanup();
				}
				break;

			// Close event
			case WM_CLOSE:
			{
				WindowEvent event;
				event.type = WindowEventType::Closed;
				mEvents.push(event);
				break;
			}
		}
	}

	void Win32Window::setTracking(bool track)
	{
		TRACKMOUSEEVENT mouseEvent;
		mouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		mouseEvent.dwFlags = track ? TME_LEAVE : TME_CANCEL;
		mouseEvent.hwndTrack = mHandle;
		mouseEvent.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&mouseEvent);
	}

	void Win32Window::grabCursor(bool grabbed)
	{
		if (grabbed)
		{
			RECT rect;
			GetClientRect(mHandle, &rect);
			MapWindowPoints(mHandle, NULL, reinterpret_cast<LPPOINT>(&rect), 2);
			ClipCursor(&rect);
		}
		else
		{
			ClipCursor(NULL);
		}
	}

	void Win32Window::cleanup()
	{
		// Restore the previous video mode (in case we were running in fullscreen)
		if (sFullscreenWindow.get() == this)
		{
			ChangeDisplaySettingsW(NULL, 0);
			sFullscreenWindow = nullptr;
		}

		// Unhide the mouse cursor (in case it was hidden)
		//setMouseCursorVisible(true);

		// No longer track the cursor
		setTracking(false);

		// No longer capture the cursor
		ReleaseCapture();
	}

	LRESULT Win32Window::globalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// Associate handle and Window instance when the creation message is received
		if (message == WM_CREATE)
		{
			// Get WindowImplWin32 instance (it was passed as the last argument of CreateWindow)
			LONG_PTR window = (LONG_PTR)reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;

			// Set as the "user data" parameter of the window
			SetWindowLongPtrW(handle, GWLP_USERDATA, window);
		}

		// Get the WindowImpl instance corresponding to the window handle
		Win32Window* window = handle ? reinterpret_cast<Win32Window*>(GetWindowLongPtr(handle, GWLP_USERDATA)) : NULL;

		// Forward the event to the appropriate function
		if (window)
		{
			window->processEvent(message, wParam, lParam);

			if (window->mCallback)
				return CallWindowProcW(reinterpret_cast<WNDPROC>(window->mCallback), handle, message, wParam, lParam);
		}

		// We don't forward the WM_CLOSE message to prevent the OS from automatically destroying the window
		if (message == WM_CLOSE)
			return 0;

		// Don't forward the menu system command, so that pressing ALT or F10 doesn't steal the focus
		if ((message == WM_SYSCOMMAND) && (wParam == SC_KEYMENU))
			return 0;

		return DefWindowProcW(handle, message, wParam, lParam);
	}

	void setProcessDpiAware()
	{
		// Try SetProcessDpiAwareness first
		DynLib* shCoreLib = ls_new<DynLib>("Shcore.dll");

		if (shCoreLib != nullptr)
		{
			enum ProcessDpiAwareness
			{
				ProcessDpiUnaware = 0,
				ProcessSystemDpiAware = 1,
				ProcessPerMonitorDpiAware = 2
			};

			typedef HRESULT(WINAPI* SetProcessDpiAwarenessFuncType)(ProcessDpiAwareness);
			SetProcessDpiAwarenessFuncType SetProcessDpiAwarenessFunc = reinterpret_cast<SetProcessDpiAwarenessFuncType>(shCoreLib->getSymbol("SetProcessDpiAwareness"));

			if (SetProcessDpiAwarenessFunc)
			{
				// We only check for E_INVALIDARG because we would get
				// E_ACCESSDENIED if the DPI was already set previously
				// and S_OK means the call was successful
				if (SetProcessDpiAwarenessFunc(ProcessSystemDpiAware) == E_INVALIDARG)
				{
					LOGERR("Failed to set process DPI awareness");
				}
				else
				{
					shCoreLib->unload();
					ls_delete(shCoreLib);
					return;
				}
			}

			shCoreLib->unload();
			ls_delete(shCoreLib);
		}

		// Fall back to SetProcessDPIAware if SetProcessDpiAwareness
		// is not available on this system
		DynLib* user32Lib = ls_new<DynLib>("user32.dll");

		if (user32Lib != nullptr)
		{
			typedef BOOL(WINAPI* SetProcessDPIAwareFuncType)(void);
			SetProcessDPIAwareFuncType SetProcessDPIAwareFunc = reinterpret_cast<SetProcessDPIAwareFuncType>(shCoreLib->getSymbol("SetProcessDPIAware"));

			if (SetProcessDPIAwareFunc)
			{
				if (!SetProcessDPIAwareFunc())
					LOGERR("Failed to set process DPI awareness");
			}

			user32Lib->unload();
			ls_delete(user32Lib);
		}
	}
}

#endif // PLATFORM_WINDOWS