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

	virtual bool init(const char* title, int x, int y, int width, int height);

	virtual void adjustWindow(int width, int height, bool windowFrame);

	virtual void clearWindow();

	virtual void closeWindow();

	/** Handle a native (Win32) windowing event as received from the system */
	virtual LRESULT handleNativeWindowingEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


public:

	virtual bool isSameKindAs(const Object* object) const { return dynamic_cast<const GraphicsWindowWin32*>(object) != 0; }
	virtual const char* libraryName() const { return "viewer"; }
	virtual const char* className() const { return "GraphicsWindowWin32"; }

	virtual bool checkEvents();

protected:

	bool registerWindowClasses();

	void unregisterWindowClasses();

	// Display devices present in the system
	typedef std::vector<DISPLAY_DEVICE> DisplayDevices;

	void enumerateDisplayDevices(DisplayDevices& displayDevices) const;

	void destroyWindow(bool deleteNativeWindow = true);

protected:

	HWND			_hwnd;

	RECT			_rect;

	DWORD			_style;

	int				_oldWidth;

	int				_oldHeight;

	int				_frameWidth;

	int				_frameHeight;

	bool			_windowClassesRegistered;

	bool			_initialized;

	bool			_closeWindow;

	bool			_frame;
};

}