#include "GraphicsContext.h"
#include "Core/Notify.h"

#include <map>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stdio.h>

namespace graphics {

GraphicsContext::GraphicsContext()
{
	setThreadSafeRefUnref(true);
}

GraphicsContext::GraphicsContext(const GraphicsContext&, const core::CopyOp&)
{
	setThreadSafeRefUnref(true);
}

GraphicsContext::~GraphicsContext()
{
	
}

}