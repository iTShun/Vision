#include "LSApplication.h"
#include "Error/LSException.h"
#include "Logger/LSLogger.h"
#include "Platform/Win32/LSWindow-win32.h"

namespace ls
{
	Application::Application()
		: mRunning(false)
	{
		// Ensure all errors are reported properly
		CrashHandler::startUp();
	}

	Application::~Application()
	{

		MemStack::endThread();

		CrashHandler::shutDown();
	}

	INT32 Application::run(int argc, const char **argv)
	{
		if (!isStartedUp())
		{
			LS_EXCEPT(InternalErrorException, "Trying to start an not already started module.");
			return -1;
		}

		mRunning = true;

		Window* wnd = ls_new<NativeWindow>(VideoMode(1280, 720), L"Hello ТоЫГ");

		while (mRunning)
		{
			WindowEvent event;
			while (wnd->pollEvent(event))
			{

			}
		}

		ls_delete(wnd);

		Application::shutDown();

		return 0;
	}

	void Application::onStartUp()
	{
		MemStack::beginThread();


	}


}