#pragma once

#include "LSCorePrerequisites.h"
#include "Math/LSVector2I.h"
#include "Math/LSRect2I.h"

namespace ls
{
	/** @addtogroup Platform-Internal
	*  @{
	*/

	/** Contains values representing default mouse cursor types. */
	enum class PlatformCursorType
	{
		Arrow,
		Wait,
		IBeam,
		Help,
		Hand,
		SizeAll,
		SizeNESW,
		SizeNS,
		SizeNWSE,
		SizeWE
	};

	/**
	* Contains values reprenting window non client areas.
	*
	* @note	These are used for things like resize/move and tell the OS where each of those areas are on our window.
	*/
	enum class NonClientAreaBorderType
	{
		TopLeft,
		Top,
		TopRight,
		Left,
		Right,
		BottomLeft,
		Bottom,
		BottomRight
	};

	/** Types of mouse buttons provided by the OS. */
	enum class OSMouseButton
	{
		Left, Middle, Right, Count
	};

	/** Describes pointer (mouse, touch) states as reported by the OS. */
	struct LS_CORE_EXPORT OSPointerButtonStates
	{
		OSPointerButtonStates()
		{
			mouseButtons[0] = false;
			mouseButtons[1] = false;
			mouseButtons[2] = false;

			shift = false;
			ctrl = false;
		}

		bool mouseButtons[(UINT32)OSMouseButton::Count];
		bool shift, ctrl;
	};

	/**	Represents a specific non client area used for window resizing. */
	struct LS_CORE_EXPORT NonClientResizeArea
	{
		NonClientAreaBorderType type;
		Rect2I area;
	};

	/** Contains a list of window move and resize non client areas. */
	struct LS_CORE_EXPORT WindowNonClientAreaData
	{
		Vector<NonClientResizeArea> resizeAreas;
		Vector<Rect2I> moveAreas;
	};

	/**	Provides access to various operating system functions, including the main message pump. */
	class LS_CORE_EXPORT Platform
	{
	public:
		struct Pimpl;

		Platform() = default;
		virtual ~Platform();
	
		/**
		* Message pump. Processes OS messages and returns when it's free.
		*
		* @note	Core thread only.
		*/
		static void _messagePump();

		/** Called during application start up from the sim thread. Must be called before any other operations are done. */
		static void _startUp();

		/** Called once per frame from the sim thread. */
		static void _update();

		/** Called once per frame from the core thread. */
		static void _coreUpdate();

		/** Called during application shut down from the sim thread. */
		static void _shutDown();

	protected:
		static Pimpl* mData;
	};

	/** @} */
}