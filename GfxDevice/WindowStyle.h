#pragma once

#include "Utility/Flags.h"
using namespace Utility;

namespace GfxDevice
{
	////////////////////////////////////////////////////////////
	/// \ingroup window
	/// \brief Enumeration of the window styles
	///
	////////////////////////////////////////////////////////////
	struct WindowStyle
	{
		enum Enum
		{
			kNone = 0,      ///< No border / title bar (this flag and all others are mutually exclusive)
			kTitlebar = 1 << 0, ///< Title bar + fixed border
			kResize = 1 << 1, ///< Title bar + resizable border + maximize button
			kClose = 1 << 2, ///< Title bar + close button
			kFullscreen = 1 << 3, ///< Fullscreen mode (this flag and all others are mutually exclusive)

			kDefault = kTitlebar | kResize | kClose ///< Default window style
		};
	};

	typedef Flags<WindowStyle::Enum> WindowStyleFlags;
	FLAGS_OPERATORS(WindowStyle::Enum)
}