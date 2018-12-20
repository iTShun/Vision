#pragma once

#include "Utility/LSFlags.h"

namespace ls
{
	////////////////////////////////////////////////////////////
	/// \ingroup window
	/// \brief Enumeration of the window styles
	///
	////////////////////////////////////////////////////////////
	enum class WindowStyle
	{
		None = 0,				///< No border / title bar (this flag and all others are mutually exclusive)
		Titlebar = 1 << 0,		///< Title bar + fixed border
		Resize = 1 << 1,		///< Title bar + resizable border + maximize button
		Close = 1 << 2,			///< Title bar + close button
		Fullscreen = 1 << 3,	///< Fullscreen mode (this flag and all others are mutually exclusive)

		Default = Titlebar | Resize | Close ///< Default window style
	};

	typedef Flags<WindowStyle> WindowStyleFlags;
	LS_FLAGS_OPERATORS(WindowStyle)

	/** Types of events that a RenderWindow can be notified of. */
	enum class WindowEventType
	{
		/** Triggered when window size changes. */
		Resized,
		/** Triggered when window position changes. */
		Moved,
		/** Triggered when window receives input focus. */
		FocusReceived,
		/** Triggered when window loses input focus. */
		FocusLost,
		/** Triggered when the window is minimized (iconified). */
		Minimized,
		/** Triggered when the window is expanded to cover the current screen. */
		Maximized,
		/** Triggered when the window leaves minimized or maximized state. */
		Restored,
		/** Triggered when the mouse pointer leaves the window area. */
		MouseLeft,
		/** Triggered when the user wants to close the window. */
		CloseRequested,
	};

	typedef Flags<WindowEventType> WindowEventTypeFlags;
	LS_FLAGS_OPERATORS(WindowEventType)
}