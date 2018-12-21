#pragma once

#include "Window/LSWindow.h"

#if PLATFORM_WINDOWS

#include <windows.h>

namespace ls
{
	class Win32Window : public Window
	{
	public:
		Win32Window();

		Win32Window(VideoMode mode, const WString& title, WindowStyleFlags style = WindowStyle::Default);

		Win32Window(WindowHandle handle);

		virtual ~Win32Window();

	public:
		virtual void create(VideoMode mode, const WString& title, WindowStyleFlags style = WindowStyle::Default) override;

		virtual void create(WindowHandle handle) override;

		virtual void close() override;

		virtual void setTitle(const WString& title) override;

		virtual void setPosition(const Vector2I& position) override;

		virtual Vector2I getPosition() const override;

		virtual void setSize(const Vector2I& size) override;

		virtual Vector2I getSize() const override;

		virtual void setVisible(bool visible) override;

		virtual bool isVisible() override;

	protected:
		virtual void processEvents() override;

	protected:
		////////////////////////////////////////////////////////////
		/// Register the window class
		///
		////////////////////////////////////////////////////////////
		void registerWindowClass();

		////////////////////////////////////////////////////////////
		/// \brief Switch to fullscreen mode
		///
		/// \param mode Video mode to switch to
		///
		////////////////////////////////////////////////////////////
		void switchToFullscreen(const VideoMode& mode);

		////////////////////////////////////////////////////////////
		/// \brief Process a Win32 event
		///
		/// \param message Message to process
		/// \param wParam  First parameter of the event
		/// \param lParam  Second parameter of the event
		///
		////////////////////////////////////////////////////////////
		void processEvent(UINT message, WPARAM wParam, LPARAM lParam);

		////////////////////////////////////////////////////////////
		/// \brief Enables or disables tracking for the mouse cursor leaving the window
		///
		/// \param track True to enable, false to disable
		///
		////////////////////////////////////////////////////////////
		void setTracking(bool track);

		////////////////////////////////////////////////////////////
		/// \brief Grab or release the mouse cursor
		///
		/// This is not to be confused with setMouseCursorGrabbed.
		/// Here m_cursorGrabbed is not modified; it is used,
		/// for example, to release the cursor when switching to
		/// another application.
		///
		/// \param grabbed True to enable, false to disable
		///
		////////////////////////////////////////////////////////////
		void grabCursor(bool grabbed);

		////////////////////////////////////////////////////////////
		/// \brief Free all the resources attached to the window
		///
		////////////////////////////////////////////////////////////
		void cleanup();

		////////////////////////////////////////////////////////////
		/// \brief Convert a Win32 virtual key code to a SFML key code
		///
		/// \param key   Virtual key code to convert
		/// \param flags Additional flags
		///
		/// \return SFML key code corresponding to the key
		///
		////////////////////////////////////////////////////////////
		//static Keyboard::Key virtualKeyCodeToSF(WPARAM key, LPARAM flags);

		////////////////////////////////////////////////////////////
		/// \brief Function called whenever one of our windows receives a message
		///
		/// \param handle  Win32 handle of the window
		/// \param message Message received
		/// \param wParam  First parameter of the message
		/// \param lParam  Second parameter of the message
		///
		/// \return True to discard the event after it has been processed
		///
		////////////////////////////////////////////////////////////
		static LRESULT CALLBACK globalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

	protected:
		////////////////////////////////////////////////////////////
		// Member data
		////////////////////////////////////////////////////////////
		HWND     mHandle;           ///< Win32 handle of the window
		LONG_PTR mCallback;         ///< Stores the original event callback function of the control
		bool     mFullscreen;       ///< Is the window fullscreen?
		bool     mResizing;         ///< Is the window being resized?
	};

	typedef Win32Window NativeWindow;
}

#endif // PLATFORM_WINDOWS