#ifndef GraphicsWindow_h
#define GraphicsWindow_h

#include "Graphics/Exports.h"
#include "Core/Object.h"

namespace graphics {

/** Base class for providing Windowing API agnostic access to creating and managing graphics window and events.
 */
class API_GRAPHICS GraphicsWindow : public core::Object
{
public:

	virtual bool isSameKindAs(const Object* object) const { return dynamic_cast<const GraphicsWindow*>(object) != 0; }
	virtual const char* libraryName() const { return "graphics"; }
	virtual const char* className() const { return "GraphicsWindow"; }

	typedef uint16_t WindowHandle;

	inline WindowHandle getWindowHandle() const { return _handle; }

	static GraphicsWindow * createGraphicsWindow(int x, int y, int width, int height, const char* title = "");

	//virtual bool createWindow(int x, int y, int width, int height, const char* title = "") { return false; }

	//virtual void destroyWindow() {}

	//virtual void setWindowPos(int x, int y) {}

	//virtual void setWindowSize(int width, int height) {}

	///** Set the name of the window */
	//virtual void setWindowName(const std::string& /*name*/) { LOG_NOTICE << "GraphicsWindow::setWindowName(..) not implemented." << std::endl; }

	///** Return the name of the window */
	//virtual std::string getWindowName() { return ""; }

	//virtual void toggleFullscreen() {}

	//virtual void setMouseLock(bool lock) {}

	//virtual void setCurrentDir(const char* dir) {}

	///** Get focus.*/
	//virtual void grabFocus() { LOG_NOTICE << "GraphicsWindow::grabFocus(..) not implemented." << std::endl; }

	///** Get focus on if the pointer is in this window.*/
	//virtual void grabFocusIfPointerInWindow() { LOG_NOTICE << "GraphicsWindow::grabFocusIfPointerInWindow(..) not implemented." << std::endl; }

public:

	/** Return whether a valid and usable GraphicsContext has been created.*/
	virtual bool valid() const { LOG_NOTICE << "GraphicsWindow::valid() not implemented." << std::endl; return _handle != UINT16_MAX; }

	virtual bool checkEvents() { return false; }

protected:

	GraphicsWindow();

	GraphicsWindow(const GraphicsWindow&, const core::CopyOp&);

	virtual Object* cloneType() const { return 0; }
	virtual Object* clone(const core::CopyOp&) const { return 0; }

	WindowHandle	_handle;

	std::string		_windowName;

	int				_x;

	int				_y;

	int				_width;

	int				_height;

	float			_aspectRatio;

	double			_refreshRate;

};

}

#endif /* GraphicsWindow_h */