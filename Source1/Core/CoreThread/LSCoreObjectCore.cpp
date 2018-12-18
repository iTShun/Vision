#include "CoreThread/LSCoreObjectCore.h"
#include "CoreThread/LSCoreThread.h"

namespace ls
{
	namespace ct 
	{
	Signal CoreObject::mCoreGpuObjectLoadedCondition;
	Mutex CoreObject::mCoreGpuObjectLoadedMutex;

	CoreObject::CoreObject()
		:mFlags(0)
	{ }

	CoreObject::~CoreObject()
	{ 
		THROW_IF_NOT_CORE_THREAD;
	}

	void CoreObject::initialize()
	{
		{
			Lock lock(mCoreGpuObjectLoadedMutex);
			setIsInitialized(true);
		}

		setScheduledToBeInitialized(false);

		mCoreGpuObjectLoadedCondition.notify_all();
	}

	void CoreObject::synchronize()
	{
		if (!isInitialized())
		{
#if DEBUG_MODE
			if (LS_THREAD_CURRENT_ID == CoreThread::instance().getCoreThreadId())
				LS_EXCEPT(InternalErrorException, "You cannot call this method on the core thread. It will cause a deadlock!");
#endif

			Lock lock(mCoreGpuObjectLoadedMutex);
			while (!isInitialized())
			{
				if (!isScheduledToBeInitialized())
					LS_EXCEPT(InternalErrorException, "Attempting to wait until initialization finishes but object is not scheduled to be initialized.");

				mCoreGpuObjectLoadedCondition.wait(lock);
			}
		}
	}

	void CoreObject::_setThisPtr(SPtr<CoreObject> ptrThis)
	{
		mThis = ptrThis;
	}
	}
}