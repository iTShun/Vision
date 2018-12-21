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

	////////////////////////////////////////////////////////////
	/// \brief Enumeration of the different types of events
	///
	////////////////////////////////////////////////////////////
	enum class WindowEventType
	{
		Closed,					///< The window requested to be closed (no data)
		Resized,                ///< The window was resized (data in event.size)
		Moved,					///< The window was position changes. (data in event.position)
		LostFocus,              ///< The window lost the focus (no data)
		GainedFocus,            ///< The window gained the focus (no data)
		Minimized,				///< The window is minimized (iconified). (no data)
		Maximized,				///< The window is expanded to cover the current screen. (no data)
		Restored,				///< The window leaves minimized or maximized state. (no data)
		TextEntered,            ///< A character was entered (data in event.text)
		KeyPressed,             ///< A key was pressed (data in event.key)
		KeyReleased,            ///< A key was released (data in event.key)
		MouseWheelMoved,        ///< The mouse wheel was scrolled (data in event.mouseWheel) (deprecated)
		MouseWheelScrolled,     ///< The mouse wheel was scrolled (data in event.mouseWheelScroll)
		MouseButtonPressed,     ///< A mouse button was pressed (data in event.mouseButton)
		MouseButtonReleased,    ///< A mouse button was released (data in event.mouseButton)
		MouseMoved,             ///< The mouse cursor moved (data in event.mouseMove)
		MouseEntered,           ///< The mouse cursor entered the area of the window (no data)
		MouseLeft,              ///< The mouse cursor left the area of the window (no data)
		JoystickButtonPressed,  ///< A joystick button was pressed (data in event.joystickButton)
		JoystickButtonReleased, ///< A joystick button was released (data in event.joystickButton)
		JoystickMoved,          ///< The joystick moved along an axis (data in event.joystickMove)
		JoystickConnected,      ///< A joystick was connected (data in event.joystickConnect)
		JoystickDisconnected,   ///< A joystick was disconnected (data in event.joystickConnect)
		TouchBegan,             ///< A touch event began (data in event.touch)
		TouchMoved,             ///< A touch moved (data in event.touch)
		TouchEnded,             ///< A touch event ended (data in event.touch)
		SensorChanged,          ///< A sensor value changed (data in event.sensor)

		Count                   ///< Keep last -- the total number of event types
	};

	typedef Flags<WindowEventType> WindowEventTypeFlags;
	LS_FLAGS_OPERATORS(WindowEventType)
}