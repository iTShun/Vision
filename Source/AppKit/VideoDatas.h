#pragma once

#include "GenericKit/GenericKit.h"

#if PLATFORM_WINDOWS
#	include <Windows.h>
#endif // PLATFORM_

namespace AppKit
{
	class VideoDevice;

	struct DisplayMode
	{
		int32_t Format;                 /**< pixel format */
		int Width;                      /**< width, in screen coordinates */
		int Height;                     /**< height, in screen coordinates */
		int RefreshRate;                /**< refresh rate (or zero for unspecified) */

		/**< driver-specific data */
#if PLATFORM_WINDOWS
		DEVMODE DeviceMode;
#elif PLATFORM_OSX
		CGDisplayModeRef ModeRef;
#endif // PLATFORM_

		DisplayMode()
		{
			Log("DisplayMode::DisplayMode()");
		}

		~DisplayMode()
		{
			Log("DisplayMode::~DisplayMode()");
		}
	};

	struct VideoDisplay
	{
		GenericKit::String Name;
		int MaxDisplaymodes;
		int NumDisplayModes;
		DisplayMode *DisplayModes;
		DisplayMode DesktopMode;
		DisplayMode CurrentMode;

		//        SDL_Window *fullscreen_window;

		VideoDevice *Device;

#if PLATFORM_WINDOWS
		TCHAR DeviceName[32];
		HMONITOR MonitorHandle;
#elif PLATFORM_OSX
		CGDirectDisplayID Display;
#endif // PLATFORM_

		VideoDisplay()
		{
			Log("VideoDisplay::VideoDisplay()");
		}

		~VideoDisplay()
		{
			Log("VideoDisplay::~VideoDisplay()");
		}
	};
}