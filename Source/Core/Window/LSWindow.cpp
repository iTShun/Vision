#include "LSWindow.h"
#include "Logger/LSLogger.h"
#include "Error/LSException.h"

namespace ls
{
	static UPtr<Window> sFullscreenWindow(nullptr, nullptr);

	////////////////////////////////////////////////////////////
	Window::Window()
	{
		
	}

	////////////////////////////////////////////////////////////
	Window::Window(VideoMode mode, const WString& title, WindowStyleFlags style)
	{
		create(mode, title, style);
	}

	////////////////////////////////////////////////////////////
	Window::Window(WindowHandle handle)
	{
		create(handle);
	}

	////////////////////////////////////////////////////////////
	Window::~Window()
	{
		close();
	}

	////////////////////////////////////////////////////////////
	void Window::create(VideoMode mode, const WString & title, WindowStyleFlags style)
	{
		// Destroy the previous window implementation
		close();

		// Fullscreen style requires some tests
		if (style & WindowStyle::Fullscreen)
		{
			// Make sure there's not already a fullscreen window (only one is allowed)
			if (sFullscreenWindow)
			{
				LOGERR("Creating two fullscreen windows is not allowed, switching to windowed mode");
				style &= ~WindowStyle::Fullscreen;
			}
			else
			{
				// Make sure that the chosen video mode is compatible
				if (!mode.isValid())
				{
					LOGERR("The requested video mode is not available, switching to a valid mode");
					mode = VideoMode::getFullscreenModes()[0];
				}

				// Update the fullscreen window
				sFullscreenWindow = ls_unique_ptr(this);
			}
		}

		// Check validity of style according to the underlying platform
#if PLATFORM_IOS || PLATFORM_ANDROID
		if (style & WindowStyle::Fullscreen)
			style &= ~WindowStyle::Titlebar;
		else
			style |= WindowStyle::Titlebar;
#else
		if ((style & WindowStyle::Close) || (style & WindowStyle::Resize))
			style |= WindowStyle::Titlebar;
#endif
	}

	////////////////////////////////////////////////////////////
	void Window::create(WindowHandle handle)
	{
		// Destroy the previous window implementation
		close();
	}

	////////////////////////////////////////////////////////////
	bool Window::pollEvent(WindowEvent & event)
	{
		if (popEvent(event, false))
			return filterEvent(event);

		return false;
	}

	////////////////////////////////////////////////////////////
	bool Window::waitEvent(WindowEvent & event)
	{
		if (popEvent(event, true))
			return filterEvent(event);

		return false;
	}

	////////////////////////////////////////////////////////////
	bool Window::popEvent(WindowEvent & event, bool block)
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
					PlatformUtil::sleep(10);
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

	////////////////////////////////////////////////////////////
	bool Window::filterEvent(const WindowEvent & event)
	{
		return true;
	}
}