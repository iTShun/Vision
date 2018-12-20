#pragma once

#include "Platform/LSPlatformConfig.h"

// Windows' HWND is a typedef on struct HWND__*
#if PLATFORM_WINDOWS
struct HWND__;
#endif

namespace ls
{
#if PLATFORM_WINDOWS
	// Window handle is HWND (HWND__*) on Windows
	typedef HWND__* WindowHandle;
#elif PLATFORM_LINUX
	// Window handle is Window (unsigned long) on Unix - X11
	typedef unsigned long WindowHandle;
#elif PLATFORM_OSX
	// Window handle is NSWindow or NSView (void*) on Mac OS X - Cocoa
	typedef void* WindowHandle;
#elif PLATFORM_IOS
	// Window handle is UIWindow (void*) on iOS - UIKit
	typedef void* WindowHandle;
#elif PLATFORM_ANDROID
	// Window handle is ANativeWindow* (void*) on Android
	typedef void* WindowHandle;
#else
	typedef void* WindowHandle;
#endif
}