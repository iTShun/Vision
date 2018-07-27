#include "GraphicsWindowWin32.h"

namespace viewer {

GraphicsWindowWin32::GraphicsWindowWin32() :
	_windowProcedure(0)
{

}

GraphicsWindowWin32::~GraphicsWindowWin32()
{
	destroyWindow();
}

bool GraphicsWindowWin32::createWindow(int x, int y, int width, int height, const char * title)
{


	return false;
}

void GraphicsWindowWin32::destroyWindow()
{
}

void GraphicsWindowWin32::setWindowPos(int x, int y)
{
}

void GraphicsWindowWin32::setWindowSize(int width, int height)
{
}

void GraphicsWindowWin32::setWindowName(const std::string &)
{
}

void GraphicsWindowWin32::toggleFullscreen()
{
}

void GraphicsWindowWin32::setMouseLock(bool lock)
{
}

void GraphicsWindowWin32::setCurrentDir(const char * dir)
{
}

void GraphicsWindowWin32::grabFocus()
{
}

void GraphicsWindowWin32::grabFocusIfPointerInWindow()
{
}



}