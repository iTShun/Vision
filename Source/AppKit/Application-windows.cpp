#include "Application.h"
using namespace GenericKit;

#if PLATFORM_WINDOWS

namespace AppKit
{
	Application::Application()
	{

	}

	Application::~Application()
	{

	}

	int32_t Application::Run(int argc, const char* const* argv)
	{

		return 0;
	}
}

#endif // PLATFORM_WINDOWS
