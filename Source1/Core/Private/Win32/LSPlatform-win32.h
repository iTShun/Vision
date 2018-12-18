#pragma once

#include "Platform/LSPlatform.h"

#if PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // required to stop windows.h messing up std::min
#endif

#include <windows.h>
#include <windowsx.h>

#define WM_LS_SETCAPTURE		WM_USER + 101
#define WM_LS_RELEASECAPTURE	WM_USER + 102

namespace ls
{
	/** @addtogroup Platform-Internal
	*  @{
	*/

	/** Various Win32 specific functionality. Contains the main message loop. */
	class LS_CORE_EXPORT Win32Platform : public Platform
	{
	public:
		/** Main message loop callback that processes messages received from windows. */
		static LRESULT CALLBACK _win32WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

	/** @} */
}

#endif // PLATFORM_WINDOWS