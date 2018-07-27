#pragma once

#include "Viewer/Exports.h"
#include "Graphics/GraphicsWindow.h"

#include <windows.h>

namespace viewer {

class API_VIEWER GraphicsWindowWin32 : public graphics::GraphicsWindow
{

public:

	GraphicsWindowWin32();

	virtual ~GraphicsWindowWin32();

public:

	virtual bool isSameKindAs(const Object* object) const { return dynamic_cast<const GraphicsWindowWin32*>(object) != 0; }
	virtual const char* libraryName() const { return "viewer"; }
	virtual const char* className() const { return "GraphicsWindowWin32"; }

public:

	virtual bool createWindow(int x, int y, int width, int height, const char* title = "");

	virtual void destroyWindow();

	virtual void setWindowPos(int x, int y);

	virtual void setWindowSize(int width, int height);

	/** Set the name of the window */
	virtual void setWindowName(const std::string& /*name*/);

	/** Return the name of the window */
	virtual std::string getWindowName() { return _title; }

	virtual void toggleFullscreen();

	virtual void setMouseLock(bool lock);

	virtual void setCurrentDir(const char* dir);

	/** Get focus.*/
	virtual void grabFocus();

	/** Get focus on if the pointer is in this window.*/
	virtual void grabFocusIfPointerInWindow();


protected:

	std::string		_title;
	WNDPROC         _windowProcedure;
};

}