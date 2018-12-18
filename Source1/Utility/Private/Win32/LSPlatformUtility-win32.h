#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"

#if PLATFORM_WINDOWS

#include <windows.h>

namespace ls
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Platform-Utility-Internal
	 *  @{
	 */

	/** Provides access to various Windows specific utility functions. */
	class LS_UTILITY_EXPORT Win32PlatformUtility
	{
	public:
		/**
		 * Creates a new bitmap usable by various Win32 methods from the provided pixel data. Caller must ensure to call 
		 * DeleteObject() on the bitmap handle when finished.
		 */
		static HBITMAP createBitmap(const Color* pixels, UINT32 width, UINT32 height, bool premultiplyAlpha);
	};

	/** @} */
	/** @} */
}

#endif // PLATFORM_WINDOWS
