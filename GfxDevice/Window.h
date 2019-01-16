#pragma once

#include "Utility/Utility.h"
#include "WindowStyle.h"
#include "WindowEvent.h"
#include "VideoMode.h"

namespace GfxDevice
{
	class Window : public NonCopyable
	{
	public:
		virtual ~Window() {}

		////////////////////////////////////////////////////////////
		/// \brief Create (or recreate) the window
		///
		/// If the window was already created, it closes it first.
		///
		/// \param mode     Video mode to use (defines the width, height and depth of the rendering area of the window)
		/// \param title    Title of the window
		/// \param style    %Window style, a bitwise OR combination of WindowStyle enumerators
		/// \param data		Platform-specific data of the control
		///
		////////////////////////////////////////////////////////////
		virtual bool create(VideoMode mode, const char* title, WindowStyleFlags style = WindowStyle::kDefault, void* data = nullptr) = 0;
		virtual bool create(VideoMode mode, const WString& title, WindowStyleFlags style = WindowStyle::kDefault, void* data = nullptr) = 0;

		////////////////////////////////////////////////////////////
		/// \brief Create (or recreate) the window from an existing control
		///
		/// If the window was already created, it closes it first.
		///
		/// \param data   Platform-specific data of the control
		///
		////////////////////////////////////////////////////////////
		virtual bool create(void* data) = 0;

		////////////////////////////////////////////////////////////
		/// \brief Close the window and destroy all the attached resources
		///
		/// After calling this function, the Window instance remains
		/// valid and you can call create() to recreate the window.
		/// All other functions such as pollEvent() or display() will
		/// still work, and will have no effect on closed windows.
		///
		////////////////////////////////////////////////////////////
		virtual void close() = 0;

		////////////////////////////////////////////////////////////
		/// \brief Pop the event on top of the event queue, if any, and return it
		///
		/// This function is not blocking: if there's no pending event then
		/// it will return false and leave \a event unmodified.
		/// Note that more than one event may be present in the event queue,
		/// thus you should always call this function in a loop
		/// to make sure that you process every pending event.
		/// \code
		/// GfxDevice::Event event;
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
		virtual bool pollEvent(Event& event) = 0;

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
		/// GfxDevice::Event event;
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
		virtual bool waitEvent(Event& event) = 0;

		////////////////////////////////////////////////////////////
		/// \brief Get the client size of the window
		///
		/// \return Size of the window, in pixels
		///
		////////////////////////////////////////////////////////////
		virtual void getSize(int* w, int* h) const = 0;

	protected:
		Queue<Event> mEvents;
	};
}