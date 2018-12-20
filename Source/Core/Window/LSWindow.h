#pragma once

#include "Window/LSVideoMode.h"
#include "Window/LSWindowFwd.h"
#include "Window/LSWindowHandle.h"
#include "Utility/LSNonCopyable.h"
#include "String/LSUnicode.h"

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
		/// \param style    %Window style, a bitwise OR combination of sf::Style enumerators
		/// \param settings Additional settings for the underlying OpenGL context
		///
		////////////////////////////////////////////////////////////
		Window(VideoMode mode, const String& title, WindowStyleFlags style = WindowStyle::Default);
		
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
		/// \param settings Additional settings for the underlying OpenGL context
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


	};
}