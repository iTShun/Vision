#pragma once

#include "Window/LSVideoMode.h"
#include "Window/LSWindowFwd.h"
#include "Window/LSWindowHandle.h"
#include "Window/LSWindowEvent.h"
#include "Utility/LSNonCopyable.h"
#include "String/LSUnicode.h"
#include "Math/LSVector2I.h"

namespace ls
{
	////////////////////////////////////////////////////////////
	/// \brief Window that serves as a target
	///
	////////////////////////////////////////////////////////////
	class Window : NonCopyable
	{
	public:
		////////////////////////////////////////////////////////////
		/// \brief Default constructor
		///
		/// This constructor doesn't actually create the window,
		/// use the other constructors or call create() to do so.
		///
		////////////////////////////////////////////////////////////
		Window();

		////////////////////////////////////////////////////////////
		/// \brief Construct a new window
		///
		/// This constructor creates the window with the size and pixel
		/// depth defined in \a mode. An optional style can be passed to
		/// customize the look and behavior of the window (borders,
		/// title bar, resizable, closable, ...). If \a style contains
		/// Style::Fullscreen, then \a mode must be a valid video mode.
		///
		/// The fourth parameter is an optional structure specifying
		/// advanced OpenGL context settings such as antialiasing,
		/// depth-buffer bits, etc.
		///
		/// \param mode     Video mode to use (defines the width, height and depth of the rendering area of the window)
		/// \param title    Title of the window
		/// \param style    %Window style, a bitwise OR combination of ls::WindowStyle enumerators
		///
		////////////////////////////////////////////////////////////
		Window(VideoMode mode, const WString& title, WindowStyleFlags style = WindowStyle::Default);
		
		////////////////////////////////////////////////////////////
		/// \brief Construct the window from an existing control
		///
		/// Use this constructor if you want to create an OpenGL
		/// rendering area into an already existing control.
		///
		/// The second parameter is an optional structure specifying
		/// advanced OpenGL context settings such as antialiasing,
		/// depth-buffer bits, etc.
		///
		/// \param handle   Platform-specific handle of the control
		///
		////////////////////////////////////////////////////////////
		explicit Window(WindowHandle handle);

		////////////////////////////////////////////////////////////
		/// \brief Destructor
		///
		/// Closes the window and frees all the resources attached to it.
		///
		////////////////////////////////////////////////////////////
		virtual ~Window();

		////////////////////////////////////////////////////////////
		/// \brief Create (or recreate) the window
		///
		/// If the window was already created, it closes it first.
		/// If \a style contains Style::Fullscreen, then \a mode
		/// must be a valid video mode.
		///
		/// The fourth parameter is an optional structure specifying
		/// advanced OpenGL context settings such as antialiasing,
		/// depth-buffer bits, etc.
		///
		/// \param mode     Video mode to use (defines the width, height and depth of the rendering area of the window)
		/// \param title    Title of the window
		/// \param style    %Window style, a bitwise OR combination of ls::WindowStyle enumerators
		///
		////////////////////////////////////////////////////////////
		virtual void create(VideoMode mode, const WString& title, WindowStyleFlags style = WindowStyle::Default);

		////////////////////////////////////////////////////////////
		/// \brief Create (or recreate) the window from an existing control
		///
		/// Use this function if you want to create an OpenGL
		/// rendering area into an already existing control.
		/// If the window was already created, it closes it first.
		///
		/// The second parameter is an optional structure specifying
		/// advanced OpenGL context settings such as antialiasing,
		/// depth-buffer bits, etc.
		///
		/// \param handle   Platform-specific handle of the control
		///
		////////////////////////////////////////////////////////////
		virtual void create(WindowHandle handle);

		////////////////////////////////////////////////////////////
		/// \brief Close the window and destroy all the attached resources
		///
		/// After calling this function, the ls::Window instance remains
		/// valid and you can call create() to recreate the window.
		/// All other functions such as pollEvent() or display() will
		/// still work (i.e. you don't have to test isOpen() every time),
		/// and will have no effect on closed windows.
		///
		////////////////////////////////////////////////////////////
		virtual void close() {}

		////////////////////////////////////////////////////////////
		/// \brief Pop the event on top of the event queue, if any, and return it
		///
		/// This function is not blocking: if there's no pending event then
		/// it will return false and leave \a event unmodified.
		/// Note that more than one event may be present in the event queue,
		/// thus you should always call this function in a loop
		/// to make sure that you process every pending event.
		/// \code
		/// ls::WindowEvent event;
		/// while (window.pollEvent(event))
		/// {
		///    // process event...
		/// }
		/// \endcode
		///
		/// \param event Event to be returned
		///
		/// \return True if an event was returned, or false if the event queue was empty
		///
		/// \see waitEvent
		///
		////////////////////////////////////////////////////////////
		bool pollEvent(WindowEvent& event);

		////////////////////////////////////////////////////////////
		/// \brief Wait for an event and return it
		///
		/// This function is blocking: if there's no pending event then
		/// it will wait until an event is received.
		/// After this function returns (and no error occurred),
		/// the \a event object is always valid and filled properly.
		/// This function is typically used when you have a thread that
		/// is dedicated to events handling: you want to make this thread
		/// sleep as long as no new event is received.
		/// \code
		/// ls::WindowEvent event;
		/// if (window.waitEvent(event))
		/// {
		///    // process event...
		/// }
		/// \endcode
		///
		/// \param event Event to be returned
		///
		/// \return False if any error occurred
		///
		/// \see pollEvent
		///
		////////////////////////////////////////////////////////////
		bool waitEvent(WindowEvent& event);

		////////////////////////////////////////////////////////////
		/// \brief Change the title of the window
		///
		/// \param title New title
		///
		/// \see setIcon
		///
		////////////////////////////////////////////////////////////
		virtual void setTitle(const WString& title) {}

		////////////////////////////////////////////////////////////
		/// \brief Change the position of the window on screen
		///
		/// This function only works for top-level windows
		/// (i.e. it will be ignored for windows created from
		/// the handle of a child window/control).
		///
		/// \param position New position, in pixels
		///
		/// \see getPosition
		///
		////////////////////////////////////////////////////////////
		virtual void setPosition(const Vector2I& position) {}

		////////////////////////////////////////////////////////////
		/// \brief Get the position of the window
		///
		/// \return Position of the window, in pixels
		///
		/// \see setPosition
		///
		////////////////////////////////////////////////////////////
		virtual Vector2I getPosition() const { return Vector2I(); }

		////////////////////////////////////////////////////////////
		/// \brief Change the size of the rendering region of the window
		///
		/// \param size New size, in pixels
		///
		/// \see getSize
		///
		////////////////////////////////////////////////////////////
		virtual void setSize(const Vector2I& size) {}

		////////////////////////////////////////////////////////////
		/// \brief Get the size of the rendering region of the window
		///
		/// The size doesn't include the titlebar and borders
		/// of the window.
		///
		/// \return Size in pixels
		///
		/// \see setSize
		///
		////////////////////////////////////////////////////////////
		virtual Vector2I getSize() const { return Vector2I(); }

		////////////////////////////////////////////////////////////
		/// \brief Show or hide the window
		///
		/// The window is shown by default.
		///
		/// \param visible True to show the window, false to hide it
		///
		////////////////////////////////////////////////////////////
		virtual void setVisible(bool visible) {}

		////////////////////////////////////////////////////////////
		/// \brief The window is show or hide
		////////////////////////////////////////////////////////////
		virtual bool isVisible() { return false; }

	protected:
		virtual void processEvents() {}
	protected:
		bool popEvent(WindowEvent& event, bool block);

		bool filterEvent(const WindowEvent& event);
	
	protected:
		Queue<WindowEvent> mEvents;
	};
}