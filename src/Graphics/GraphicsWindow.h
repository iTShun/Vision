#ifndef GraphicsWindow_h
#define GraphicsWindow_h

#include "Graphics/GraphicsContext.h"

namespace graphics {

struct WindowHandle { uint16_t idx; };

/** Base class for providing Windowing API agnostic access to creating and managing graphics window and events.
* Note, the GraphicsWindow is subclassed from graphics::GraphicsContext, and to provide an implementation you'll need to implement its
* range of pure virtual functions, you'll find these all have naming convention methodNameImplementation(..).
* GraphicsWindow adds the event queue on top of the GraphicsContext, thereby adding a mechanism for adapting Windowing events
* as well as basics graphics context work, you should wire up custom GraphicsWindowImplementation to push their events through
* into the EventQueue. */
class API_GRAPHICS GraphicsWindow : public GraphicsContext
{
public:

	GraphicsWindow();

	virtual bool isSameKindAs(const Object* object) const { return dynamic_cast<const GraphicsWindow*>(object) != 0; }
	virtual const char* libraryName() const { return "graphics"; }
	virtual const char* className() const { return "GraphicsWindow"; }

	virtual bool createWindow(int x, int y, int width, int height, const char* title = "") { return false; }

	virtual void destroyWindow() {}

	virtual void setWindowPos(int x, int y) {}

	virtual void setWindowSize(int width, int height) {}

	/** Set the name of the window */
	virtual void setWindowName(const std::string& /*name*/) { LOG_NOTICE << "GraphicsWindow::setWindowName(..) not implemented." << std::endl; }

	/** Return the name of the window */
	virtual std::string getWindowName() { return ""; }

	virtual void toggleFullscreen() {}

	virtual void setMouseLock(bool lock) {}

	virtual void setCurrentDir(const char* dir) {}

	/** Get focus.*/
	virtual void grabFocus() { LOG_NOTICE << "GraphicsWindow::grabFocus(..) not implemented." << std::endl; }

	/** Get focus on if the pointer is in this window.*/
	virtual void grabFocusIfPointerInWindow() { LOG_NOTICE << "GraphicsWindow::grabFocusIfPointerInWindow(..) not implemented." << std::endl; }

public:

	/** Return whether a valid and usable GraphicsContext has been created.*/
	virtual bool valid() const { LOG_NOTICE << "GraphicsWindow::valid() not implemented." << std::endl; return _handle.idx != UINT16_MAX; }

protected:

	WindowHandle _handle;
};

}

#endif /* GraphicsWindow_h */