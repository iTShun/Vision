#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"

/** Maximum number of color surfaces that can be attached to a multi render target. */
#define LS_MAX_MULTIPLE_RENDER_TARGETS 8
#define LS_FORCE_SINGLETHREADED_RENDERING 0

/** Maximum number of individual GPU queues, per type. */
#define LS_MAX_QUEUES_PER_TYPE 8

/** Maximum number of hardware devices usable at once. */
#define LS_MAX_DEVICES 5U

/** Maximum number of devices one resource can exist at the same time. */
#define LS_MAX_LINKED_DEVICES 4U

// DLL export
#define LS_CORE_EXPORT	LS_UTILITY_EXPORT
#define LS_CORE_HIDDEN	LS_UTILITY_HIDDEN

namespace ls
{
	class CoreObjectManager;
	class CoreObject;
}

namespace ls
{
	/** Banshee thread policy that performs special startup/shutdown on threads managed by thread pool. */
	class LS_CORE_EXPORT ThreadBansheePolicy
	{
	public:
		static void onThreadStarted(const String& name)
		{
			MemStack::beginThread();
		}

		static void onThreadEnded(const String& name)
		{
			MemStack::endThread();
		}
	};

	/** Used for marking a CoreObject dependency as dirty. */
	static constexpr INT32 DIRTY_DEPENDENCY_MASK = 1 << 31;
}

#include "Utility/LSCommonTypes.h"