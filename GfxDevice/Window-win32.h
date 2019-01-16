#pragma once

#include "Window.h"

#if PLATFORM_WINDOWS

#include <Windows.h>

namespace GfxDevice
{
	class WindowWin32 : public Window
	{
	public:
		WindowWin32();
		WindowWin32(VideoMode mode, const char* title, WindowStyleFlags style = WindowStyle::kDefault, void* data = nullptr);
		WindowWin32(VideoMode mode, const WString& title, WindowStyleFlags style = WindowStyle::kDefault, void* data = nullptr);
		virtual ~WindowWin32();

	public:
		virtual bool create(VideoMode mode, const char* title, WindowStyleFlags style = WindowStyle::kDefault, void* data = nullptr) override;

		virtual bool create(VideoMode mode, const WString& title, WindowStyleFlags style = WindowStyle::kDefault, void* data = nullptr) override;

		virtual bool create(void* data) override;

		virtual void close() override;

		virtual bool pollEvent(Event& event) override;

		virtual bool waitEvent(Event& event) override;

		virtual void getSize(int* w, int* h) const override;
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
		/// \brief Free all the graphical resources attached to the window
		///
		////////////////////////////////////////////////////////////
		void cleanup();

		////////////////////////////////////////////////////////////
		/// \brief Return the next window event available
		///
		/// If there's no event available, this function calls the
		/// window's internal event processing function.
		/// The \a block parameter controls the behavior of the function
		/// if no event is available: if it is true then the function
		/// doesn't return until a new event is triggered; otherwise it
		/// returns false to indicate that no event is available.
		///
		/// \param event Event to be returned
		/// \param block Use true to block the thread until an event arrives
		///
		////////////////////////////////////////////////////////////
		bool popEvent(Event& event, bool block);

		////////////////////////////////////////////////////////////
		/// \brief Process incoming events from the operating system
		///
		////////////////////////////////////////////////////////////
		void processEvents();

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

	private:
		////////////////////////////////////////////////////////////
		// Member data
		////////////////////////////////////////////////////////////
		HWND		mHandle;			///< Win32 handle of the window
		LONG_PTR	mCallback;			///< Stores the original event callback function of the control
		bool		mCursorVisible;		///< Is the cursor visible or hidden?
		UINT32		mLastWidth, mLastHeight; ///< The last handled size of the window
		bool		mFullscreen;		///< Is the window fullscreen?
		bool		mResizing;			///< Is the window being resized?
		bool		mCursorGrabbed;		///< Is the mouse cursor trapped?
	};

	typedef WindowWin32 NativeWindow;
}

#endif