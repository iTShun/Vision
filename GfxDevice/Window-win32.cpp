#include "Window-win32.h"

#if PLATFORM_WINDOWS

// dbt.h is lowercase here, as a cross-compile on linux with mingw-w64
// expects lowercase, and a native compile on windows, whether via msvc
// or mingw-w64 addresses files in a case insensitive manner.
#include <dbt.h>

namespace GfxDevice
{
	static UINT32 sWindowCount = 0;
	static LPCWSTR sAppname = L"Win32_Window";
	static UPtr<WindowWin32> sFullscreenWindow(nullptr, nullptr);

	const GUID GUID_DEVINTERFACE_HID = { 0x4d1e55b2, 0xf16f, 0x11cf,{ 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

	void setProcessDpiAware();

	WindowWin32::WindowWin32()
		: mHandle(NULL)
		, mCallback(0)
		, mCursorVisible(true) // might need to call GetCursorInfo
		, mLastWidth(0)
		, mLastHeight(0)
		, mFullscreen(false)
		, mResizing(true)
		, mCursorGrabbed(false)
	{

	}

	WindowWin32::WindowWin32(VideoMode mode, const char * title, WindowStyleFlags style, void * data)
		: mHandle(NULL)
		, mCallback(0)
		, mCursorVisible(true) // might need to call GetCursorInfo
		, mLastWidth(mode.width)
		, mLastHeight(mode.height)
		, mFullscreen(false)
		, mResizing(true)
		, mCursorGrabbed(false)
	{
		create(mode, title, style, data);
	}

	WindowWin32::WindowWin32(VideoMode mode, const WString & title, WindowStyleFlags style, void * data)
		: mHandle(NULL)
		, mCallback(0)
		, mCursorVisible(true) // might need to call GetCursorInfo
		, mLastWidth(mode.width)
		, mLastHeight(mode.height)
		, mFullscreen(false)
		, mResizing(true)
		, mCursorGrabbed(false)
	{
		create(mode, title, style, data);
	}

	WindowWin32::~WindowWin32()
	{
		close();
	}

	bool WindowWin32::create(VideoMode mode, const char * title, WindowStyleFlags style, void * data)
	{
		return create(mode, UTF8::toWide(title), style, data);
	}

	bool WindowWin32::create(VideoMode mode, const WString & title, WindowStyleFlags style, void * data)
	{
		do
		{
			// Destroy the previous window implementation
			close();

			// Fullscreen style requires some tests
			if (style & WindowStyle::kFullscreen)
			{
				// Make sure there's not already a fullscreen window (only one is allowed)
				if (sFullscreenWindow.get())
				{
					LOGWRN("Creating two fullscreen windows is not allowed, switching to windowed mode");
					style &= ~WindowStyle::kFullscreen;
				}
				else
				{
					// Make sure that the chosen video mode is compatible
					if (!mode.isValid())
					{
						LOGWRN("The requested video mode is not available, switching to a valid mode");
						mode = VideoMode::getFullscreenModes()[0];
					}

					// Update the fullscreen window
					sFullscreenWindow = UniquePtr(this);
				}
			}

			if ((style & WindowStyle::kClose) || (style & WindowStyle::kResize))
				style |= WindowStyle::kTitlebar;

			HWND parent = (HWND)data;

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

			mLastWidth = mode.width;
			mLastHeight = mode.height;
			mFullscreen = style & WindowStyle::kFullscreen;
			mCursorGrabbed = mFullscreen;

			// Choose the window style according to the Style parameter
			DWORD win32Style = WS_VISIBLE;
			if (style == WindowStyle::kNone)
			{
				win32Style |= WS_POPUP;
			}
			else
			{
				if (style & WindowStyle::kTitlebar) win32Style |= WS_CAPTION | WS_MINIMIZEBOX;
				if (style & WindowStyle::kResize)   win32Style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
				if (style & WindowStyle::kClose)    win32Style |= WS_SYSMENU;
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
			mHandle = CreateWindowW(sAppname, title.c_str(), win32Style, left, top, width, height, parent, NULL, GetModuleHandle(NULL), this);
			if (!mHandle) {
				LOGERR(StringFormat::format("Window creation failed: {0}!", GetLastError()));
				break;
			}

			// Register to receive device interface change notifications (used for joystick connection handling)
			DEV_BROADCAST_DEVICEINTERFACE deviceInterface = { sizeof(DEV_BROADCAST_DEVICEINTERFACE), DBT_DEVTYP_DEVICEINTERFACE, 0, GUID_DEVINTERFACE_HID, 0 };
			RegisterDeviceNotification(mHandle, &deviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

			// Increment window count
			sWindowCount++;

			return true;
		} while (false);

		return false;
	}

	bool WindowWin32::create(void * data)
	{
		do
		{

			return true;
		} while (false);

		return false;
	}

	void WindowWin32::close()
	{
		// TODO should we restore the cursor shape and visibility?

		// Destroy the custom icon, if any
		//if (mIcon)
		//	DestroyIcon(mIcon);

		// If it's the last window handle we have to poll for joysticks again
		if (mHandle)
		{
			//--handleCount;

			//if (handleCount == 0)
			//	JoystickImpl::setLazyUpdates(false);
		}

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
				UnregisterClassW(sAppname, GetModuleHandleW(NULL));
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

	bool WindowWin32::pollEvent(Event & event)
	{
		return popEvent(event, false);
	}

	bool WindowWin32::waitEvent(Event & event)
	{
		return popEvent(event, true);
	}

	void WindowWin32::getSize(int * w, int * h) const
	{
		RECT rect;
		GetClientRect(mHandle, &rect);

		*w = (rect.right - rect.left);
		*h = (rect.bottom - rect.top);
	}

	void WindowWin32::registerWindowClass()
	{
		WNDCLASSW windowClass;
		windowClass.style = 0;
		windowClass.lpfnWndProc = &WindowWin32::globalOnEvent;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = GetModuleHandleW(NULL);
		windowClass.hIcon = NULL;
		windowClass.hCursor = 0;
		windowClass.hbrBackground = 0;
		windowClass.lpszMenuName = NULL;
		windowClass.lpszClassName = sAppname;
		if (!RegisterClassW(&windowClass)) {
			EXCEPT(InternalErrorException, "Window register class failed!");
		}
	}

	void WindowWin32::switchToFullscreen(const VideoMode & mode)
	{
		// Todo
	}

	void WindowWin32::cleanup()
	{
		// Restore the previous video mode (in case we were running in fullscreen)
		if (sFullscreenWindow.get() == this)
		{
			ChangeDisplaySettingsW(NULL, 0);
			sFullscreenWindow = nullptr;
		}

		// No longer capture the cursor
		ReleaseCapture();
	}

	bool WindowWin32::popEvent(Event & event, bool block)
	{
		// If the event queue is empty, let's first check if new events are available from the OS
		if (mEvents.empty())
		{
			// Get events from the system
			processEvents();

			// In blocking mode, we must process events until one is triggered
			if (block)
			{
				// Here we use a manual wait loop instead of the optimized
				// wait-event provided by the OS, so that we don't skip joystick
				// events (which require polling)
				while (mEvents.empty())
				{
					THREAD_SLEEP(10);
					processEvents();
				}
			}
		}

		// Pop the first event of the queue, if it is not empty
		if (!mEvents.empty())
		{
			event = mEvents.front();
			mEvents.pop();

			return true;
		}

		return false;
	}

	void WindowWin32::processEvents()
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

	void WindowWin32::processEvent(UINT message, WPARAM wParam, LPARAM lParam)
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
				break;
			}

			// Set cursor event
			case WM_SETCURSOR:
			{
				// The mouse has moved, if the cursor is in our window we must refresh the cursor
				if (LOWORD(lParam) == HTCLIENT)
					int d = 0;// SetCursor(mLastCursor);

				break;
			}

			// Close event
			case WM_CLOSE:
			{
				Event event;
				event.type = Event::kClosed;
				mEvents.push(event);
				break;
			}

			// Resize event
			case WM_SIZE:
			{
				int w, h;
				getSize(&w, &h);

				// Consider only events triggered by a maximize or a un-maximize
				if (wParam != SIZE_MINIMIZED && !mResizing && (mLastWidth != w || mLastHeight != h))
				{
					// Update the last handled size
					mLastWidth = w;
					mLastHeight = h;

					// Push a resize event
					Event event;
					event.type = Event::kResized;
					event.size.width = mLastWidth;
					event.size.height = mLastHeight;
					mEvents.push(event);

					// Restore/update cursor grabbing
					grabCursor(mCursorGrabbed);

					LOGDBG(StringFormat::format("WM_SIZE: {0}/{1}", w, h));
				}
				break;
			}

			// Start resizing
			case WM_ENTERSIZEMOVE:
			{
				mResizing = true;
				grabCursor(false);
				break;
			}

			// Stop resizing
			case WM_EXITSIZEMOVE:
			{
				mResizing = false;

				// Ignore cases where the window has only been moved
				int w, h;
				getSize(&w, &h);

				if (mLastWidth != w || mLastHeight != h)
				{
					// Update the last handled size
					mLastWidth = w;
					mLastHeight = h;

					// Push a resize event
					Event event;
					event.type = Event::kResized;
					event.size.width = mLastWidth;
					event.size.height = mLastHeight;
					mEvents.push(event);
				}
				LOGDBG(StringFormat::format("WM_EXITSIZEMOVE: {0}/{1}", w, h));
				// Restore/update cursor grabbing
				grabCursor(mCursorGrabbed);
				break;
			}

			// The system request the min/max window size and position
			case WM_GETMINMAXINFO:
			{
				// We override the returned information to remove the default limit
				// (the OS doesn't allow windows bigger than the desktop by default)
				MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);
				//info->ptMinTrackSize.x = 64;
				//info->ptMinTrackSize.y = 64;
				info->ptMaxTrackSize.x = 50000;
				info->ptMaxTrackSize.y = 50000;
				break;
			}

			// Gain focus event
			case WM_SETFOCUS:
			{
				// Restore cursor grabbing
				grabCursor(mCursorGrabbed);

				Event event;
				event.type = Event::kGainedFocus;
				mEvents.push(event);
				break;
			}

			// Lost focus event
			case WM_KILLFOCUS:
			{
				// Ungrab the cursor
				grabCursor(false);

				Event event;
				event.type = Event::kLostFocus;
				mEvents.push(event);
				break;
			}

			// Text event
			case WM_CHAR:
			{
				break;
			}


		}
	}

	void WindowWin32::setTracking(bool track)
	{
		TRACKMOUSEEVENT mouseEvent;
		mouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		mouseEvent.dwFlags = track ? TME_LEAVE : TME_CANCEL;
		mouseEvent.hwndTrack = mHandle;
		mouseEvent.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&mouseEvent);
	}

	void WindowWin32::grabCursor(bool grabbed)
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

	LRESULT WindowWin32::globalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
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
		WindowWin32* window = handle ? reinterpret_cast<WindowWin32*>(GetWindowLongPtr(handle, GWLP_USERDATA)) : nullptr;

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
		HINSTANCE shCoreDll = LoadLibrary(L"Shcore.dll");

		if (shCoreDll)
		{
			enum ProcessDpiAwareness
			{
				ProcessDpiUnaware = 0,
				ProcessSystemDpiAware = 1,
				ProcessPerMonitorDpiAware = 2
			};

			typedef HRESULT(WINAPI* SetProcessDpiAwarenessFuncType)(ProcessDpiAwareness);
			SetProcessDpiAwarenessFuncType SetProcessDpiAwarenessFunc = reinterpret_cast<SetProcessDpiAwarenessFuncType>(GetProcAddress(shCoreDll, "SetProcessDpiAwareness"));

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
					FreeLibrary(shCoreDll);
					return;
				}
			}

			FreeLibrary(shCoreDll);
		}

		// Fall back to SetProcessDPIAware if SetProcessDpiAwareness
		// is not available on this system
		HINSTANCE user32Dll = LoadLibrary(L"user32.dll");

		if (user32Dll)
		{
			typedef BOOL(WINAPI* SetProcessDPIAwareFuncType)(void);
			SetProcessDPIAwareFuncType SetProcessDPIAwareFunc = reinterpret_cast<SetProcessDPIAwareFuncType>(GetProcAddress(user32Dll, "SetProcessDPIAware"));

			if (SetProcessDPIAwareFunc)
			{
				if (!SetProcessDPIAwareFunc())
					LOGERR("Failed to set process DPI awareness");
			}

			FreeLibrary(user32Dll);
		}
	}

	////////////////////////////////////////////////////////////
	VideoMode VideoMode::getDesktopMode()
	{
		// Directly forward to the OS-specific implementation
		DEVMODE win32Mode;
		win32Mode.dmSize = sizeof(win32Mode);
		win32Mode.dmDriverExtra = 0;
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &win32Mode);

		return VideoMode(win32Mode.dmPelsWidth, win32Mode.dmPelsHeight, win32Mode.dmBitsPerPel);
	}

	////////////////////////////////////////////////////////////
	const Vector<VideoMode>& VideoMode::getFullscreenModes()
	{
		static Vector<VideoMode> modes;

		// Populate the array on first call
		if (modes.empty())
		{
			// Enumerate all available video modes for the primary display adapter
			DEVMODE win32Mode;
			win32Mode.dmSize = sizeof(win32Mode);
			win32Mode.dmDriverExtra = 0;
			for (int count = 0; EnumDisplaySettings(NULL, count, &win32Mode); ++count)
			{
				// Convert to ls::VideoMode
				VideoMode mode(win32Mode.dmPelsWidth, win32Mode.dmPelsHeight, win32Mode.dmBitsPerPel);

				// Add it only if it is not already in the array
				if (std::find(modes.begin(), modes.end(), mode) == modes.end())
					modes.push_back(mode);
			}

			std::sort(modes.begin(), modes.end(), std::greater<VideoMode>());
		}

		return modes;
	}
}

#endif


