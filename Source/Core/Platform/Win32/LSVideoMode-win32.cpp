#include "Window/LSVideoMode.h"

#if PLATFORM_WINDOWS

#include <windows.h>

namespace ls
{
	////////////////////////////////////////////////////////////
	VideoMode VideoMode::getDesktopMode()
	{
		// Directly forward to the OS-specific implementation
		DEVMODE win32Mode;
		win32Mode.dmSize = sizeof(win32Mode);
		win32Mode.dmDriverExtra = 0;
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &win32Mode);

		return VideoMode(win32Mode.dmPelsWidth, win32Mode.dmPelsHeight, win32Mode.dmBitsPerPel);
	}

	////////////////////////////////////////////////////////////
	const Vector<VideoMode>& VideoMode::getFullscreenModes()
	{
		static Vector<VideoMode> modes;

		// Populate the array on first call
		if (modes.empty())
		{
			// Enumerate all available video modes for the primary display adapter
			DEVMODE win32Mode;
			win32Mode.dmSize = sizeof(win32Mode);
			win32Mode.dmDriverExtra = 0;
			for (int count = 0; EnumDisplaySettings(NULL, count, &win32Mode); ++count)
			{
				// Convert to sf::VideoMode
				VideoMode mode(win32Mode.dmPelsWidth, win32Mode.dmPelsHeight, win32Mode.dmBitsPerPel);

				// Add it only if it is not already in the array
				if (std::find(modes.begin(), modes.end(), mode) == modes.end())
					modes.push_back(mode);
			}

			std::sort(modes.begin(), modes.end(), std::greater<VideoMode>());
		}

		return modes;
	}
}

#endif // PLATFORM_WINDOWS