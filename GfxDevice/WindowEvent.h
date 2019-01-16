#pragma once

namespace GfxDevice
{
	// hat_state Joystick hat states
	struct JoystickState
	{
		enum Enum
		{
			kCentered = 0,
			kUp = 1,
			kRight = 2,
			kDown = 4,
			kLeft = 8,
			kRightUp = (kRight | kUp),
			kRightDown = (kRight | kDown),
			kLeftUp = (kLeft | kUp),
			kLeftDown = (kLeft | kDown),
		};

	};

	// Keyboard keys
	struct Keyboard
	{
		enum Enum
		{
			kUnknown = -1,

			/* Printable keys */
			kSpace = 32,
			kApostrophe = 39, /* ' */
			kComma = 44, /* , */
			kMinus = 45, /* - */
			kPeriod = 46, /* . */
			kSlash = 47, /* / */
			k0 = 48,
			k1 = 49,
			k2 = 50,
			k3 = 51,
			k4 = 52,
			k5 = 53,
			k6 = 54,
			k7 = 55,
			k8 = 56,
			k9 = 57,
			kSemicolon = 59, /* ; */
			kEqual = 61, /* = */
			kA = 65,
			kB = 66,
			kC = 67,
			kD = 68,
			kE = 69,
			kF = 70,
			kG = 71,
			kH = 72,
			kI = 73,
			kJ = 74,
			kK = 75,
			kL = 76,
			kM = 77,
			kN = 78,
			kO = 79,
			kP = 80,
			kQ = 81,
			kR = 82,
			kS = 83,
			kT = 84,
			kU = 85,
			kV = 86,
			kW = 87,
			kX = 88,
			kY = 89,
			kZ = 90,
			kLeftBracket = 91, /* [ */
			kBackslash = 92, /* \ */
			kRightBracket = 93, /* ] */
			kGraveAccent = 96, /* ` */
			kWorld1 = 161, /* non-US #1 */
			kWorld2 = 162, /* non-US #2 */

			/* Function keys */
			kEscape = 256,
			kEnter = 257,
			kTab = 258,
			kBackSpace = 259,
			kInsert = 260,
			kDelete = 261,
			kRight = 262,
			kLeft = 263,
			kDown = 264,
			kUp = 265,
			kPageUp = 266,
			kPageDown = 267,
			kHome = 268,
			kEnd = 269,
			kCapsLock = 280,
			kScrollLock = 281,
			kNumLock = 282,
			kPrintScreen = 283,
			kPause = 284,
			kF1 = 290,
			kF2 = 291,
			kF3 = 292,
			kF4 = 293,
			kF5 = 294,
			kF6 = 295,
			kF7 = 296,
			kF8 = 297,
			kF9 = 298,
			kF10 = 299,
			kF11 = 300,
			kF12 = 301,
			kF13 = 302,
			kF14 = 303,
			kF15 = 304,
			kF16 = 305,
			kF17 = 306,
			kF18 = 307,
			kF19 = 308,
			kF20 = 309,
			kF21 = 310,
			kF22 = 311,
			kF23 = 312,
			kF24 = 313,
			kF25 = 314,
			kKp0 = 320,
			kKp1 = 321,
			kKp2 = 322,
			kKp3 = 323,
			kKp4 = 324,
			kKp5 = 325,
			kKp6 = 326,
			kKp7 = 327,
			kKp8 = 328,
			kKp9 = 329,
			kKpDecimal = 330,
			kKpDivide = 331,
			kKpMultiply = 332,
			kKpSubtract = 333,
			kKpAdd = 334,
			kKpEnter = 335,
			kKpEqual = 336,
			kLeftShift = 340,
			kLeftControl = 341,
			kLeftAlt = 342,
			kLeftSuper = 343,
			kRightShift = 344,
			kRightControl = 345,
			kRightAlt = 346,
			kRightSuper = 347,
			kMenu = 348,

			kLast = kMenu
		};
	};

	////////////////////////////////////////////////////////////
	/// \brief Defines a system event and its parameters
	///
	////////////////////////////////////////////////////////////
	class Event
	{
	public:
		////////////////////////////////////////////////////////////
		/// \brief Size events parameters (Resized)
		///
		////////////////////////////////////////////////////////////
		struct SizeEvent
		{
			unsigned int width;  ///< New width, in pixels
			unsigned int height; ///< New height, in pixels
		};

		////////////////////////////////////////////////////////////
		/// \brief Text event parameters (TextEntered)
		///
		////////////////////////////////////////////////////////////
		struct TextEvent
		{
			UINT32 unicode; ///< UTF-32 Unicode value of the character
		};

		////////////////////////////////////////////////////////////
		/// \brief Mouse move event parameters (MouseMoved)
		///
		////////////////////////////////////////////////////////////
		struct MouseMoveEvent
		{
			int x; ///< X position of the mouse pointer, relative to the left of the owner window
			int y; ///< Y position of the mouse pointer, relative to the top of the owner window
		};

		////////////////////////////////////////////////////////////
		/// \brief Enumeration of the different types of events
		///
		////////////////////////////////////////////////////////////
		enum EventType
		{
			kClosed,                 ///< The window requested to be closed (no data)
			kResized,                ///< The window was resized (data in event.size)
			kLostFocus,              ///< The window lost the focus (no data)
			kGainedFocus,            ///< The window gained the focus (no data)
			kTextEntered,            ///< A character was entered (data in event.text)
			kKeyPressed,             ///< A key was pressed (data in event.key)
			kKeyReleased,            ///< A key was released (data in event.key)
			kMouseWheelMoved,        ///< The mouse wheel was scrolled (data in event.mouseWheel) (deprecated)
			kMouseWheelScrolled,     ///< The mouse wheel was scrolled (data in event.mouseWheelScroll)
			kMouseButtonPressed,     ///< A mouse button was pressed (data in event.mouseButton)
			kMouseButtonReleased,    ///< A mouse button was released (data in event.mouseButton)
			kMouseMoved,             ///< The mouse cursor moved (data in event.mouseMove)
			kMouseEntered,           ///< The mouse cursor entered the area of the window (no data)
			kMouseLeft,              ///< The mouse cursor left the area of the window (no data)
			kJoystickButtonPressed,  ///< A joystick button was pressed (data in event.joystickButton)
			kJoystickButtonReleased, ///< A joystick button was released (data in event.joystickButton)
			kJoystickMoved,          ///< The joystick moved along an axis (data in event.joystickMove)
			kJoystickConnected,      ///< A joystick was connected (data in event.joystickConnect)
			kJoystickDisconnected,   ///< A joystick was disconnected (data in event.joystickConnect)
			kTouchBegan,             ///< A touch event began (data in event.touch)
			kTouchMoved,             ///< A touch moved (data in event.touch)
			kTouchEnded,             ///< A touch event ended (data in event.touch)
			kSensorChanged,          ///< A sensor value changed (data in event.sensor)

			kCount                   ///< Keep last -- the total number of event types
		};

		////////////////////////////////////////////////////////////
		// Member data
		////////////////////////////////////////////////////////////
		EventType type; ///< Type of the event

		union
		{
			SizeEvent             size;              ///< Size event parameters (Event::kResized)
			//KeyEvent              key;               ///< Key event parameters (Event::kKeyPressed, Event::kKeyReleased)
			TextEvent             text;              ///< Text event parameters (Event::kTextEntered)
			MouseMoveEvent        mouseMove;         ///< Mouse move event parameters (Event::kMouseMoved)
			//MouseButtonEvent      mouseButton;       ///< Mouse button event parameters (Event::kMouseButtonPressed, Event::kMouseButtonReleased)
			//MouseWheelEvent       mouseWheel;        ///< Mouse wheel event parameters (Event::kMouseWheelMoved) (deprecated)
			//MouseWheelScrollEvent mouseWheelScroll;  ///< Mouse wheel event parameters (Event::kMouseWheelScrolled)
			//JoystickMoveEvent     joystickMove;      ///< Joystick move event parameters (Event::kJoystickMoved)
			//JoystickButtonEvent   joystickButton;    ///< Joystick button event parameters (Event::kJoystickButtonPressed, Event::kJoystickButtonReleased)
			//JoystickConnectEvent  joystickConnect;   ///< Joystick (dis)connect event parameters (Event::kJoystickConnected, Event::kJoystickDisconnected)
			//TouchEvent            touch;             ///< Touch events parameters (Event::kTouchBegan, Event::kTouchMoved, Event::kTouchEnded)
			//SensorEvent           sensor;            ///< Sensor event parameters (Event::kSensorChanged)
		};
	};
}