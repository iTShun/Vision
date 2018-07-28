#include "GraphicsWindow.h"

namespace graphics {

GraphicsWindow::GraphicsWindow() :
	_handle(UINT16_MAX),
	_x(0),
	_y(0),
	_width(0),
	_height(0),
	_aspectRatio(.0f),
	_refreshRate(0)
{

}

}