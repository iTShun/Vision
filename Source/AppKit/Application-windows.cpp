#include "Application.h"
using namespace GenericKit;

#if PLATFORM_WINDOWS

#include <Windows.h>
#include "AppKit/VideoDevice.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace AppKit
{
	LPTSTR G_Appname = NULL;
	uint32_t G_Appstyle = 0;
	HINSTANCE G_Instance = NULL;

	Application::Application()
	: Device(nullptr)
	{

	}

	Application::~Application()
	{

	}

	int32_t Application::Run(int argc, const char* const* argv)
	{
		if (!RegisterApp())
			return -1;

		DestroyVideoDevice();

		Device = VideoDevice::Create();

		Device->MakeWindow("Hello", 0, 0, 960, 640, 0);

		return 0;
	}

	bool Application::RegisterApp()
	{
		do {
			WNDCLASSEX wcex;
			TCHAR path[MAX_PATH];
			
			if (!G_Appname)
			{
				G_Appname = "AppWindows";
#if defined(CS_BYTEALIGNCLIENT) || defined(CS_OWNDC)
				G_Appstyle = (CS_BYTEALIGNCLIENT | CS_OWNDC);
#endif
				G_Instance = GetModuleHandle(NULL);
			}

			/* Register the application class */
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.hCursor = NULL;
			wcex.hIcon = NULL;
			wcex.hIconSm = NULL;
			wcex.lpszMenuName = NULL;
			wcex.lpszClassName = G_Appname;
			wcex.style = G_Appstyle;
			wcex.hbrBackground = NULL;
			wcex.lpfnWndProc = WindowProc;
			wcex.hInstance = G_Instance;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;

			/*hint = SDL_GetHint(SDL_HINT_WINDOWS_INTRESOURCE_ICON);
			if (hint && *hint) {
				wcex.hIcon = LoadIcon(SDL_Instance, MAKEINTRESOURCE(SDL_atoi(hint)));

				hint = SDL_GetHint(SDL_HINT_WINDOWS_INTRESOURCE_ICON_SMALL);
				if (hint && *hint) {
					wcex.hIconSm = LoadIcon(SDL_Instance, MAKEINTRESOURCE(SDL_atoi(hint)));
				}
			}
			else {*/
				/* Use the first icon as a default icon, like in the Explorer */
				GetModuleFileName(G_Instance, path, MAX_PATH);
				ExtractIconEx(path, 0, &wcex.hIcon, &wcex.hIconSm, 1);
			//}

			if (!RegisterClassEx(&wcex)) {
				Log("Couldn't register application class");
				break;
			}

			return true;
		} while (false);

		return false;
	}

	void Application::DestroyVideoDevice()
	{
		if (Device)
		{
			delete Device;
			Device = nullptr;
		}
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	return CallWindowProc(DefWindowProc, hwnd, msg, wParam, lParam);
}

#endif // PLATFORM_WINDOWS
