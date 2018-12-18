#include "CoreThread/LSCoreThreadQueue.h"
#include "CoreThread/LSCommandQueue.h"
//#include "Material/LSMaterial.h"Todo
#include "CoreThread/LSCoreThread.h"

namespace ls
{
	CoreThreadQueueBase::CoreThreadQueueBase(CommandQueueBase* commandQueue)
		:mCommandQueue(commandQueue)
	{

	}

	CoreThreadQueueBase::~CoreThreadQueueBase()
	{
		ls_delete(mCommandQueue);
	}

	AsyncOp CoreThreadQueueBase::queueReturnCommand(std::function<void(AsyncOp&)> commandCallback)
	{
		return mCommandQueue->queueReturn(commandCallback);
	}

	void CoreThreadQueueBase::queueCommand(std::function<void()> commandCallback)
	{
		mCommandQueue->queue(commandCallback);
	}

	void CoreThreadQueueBase::submitToCoreThread(bool blockUntilComplete)
	{
		Queue<QueuedCommand>* commands = mCommandQueue->flush();

		gCoreThread().queueCommand(std::bind(&CommandQueueBase::playback, mCommandQueue, commands), 
			CTQF_InternalQueue | CTQF_BlockUntilComplete);
	}

	void CoreThreadQueueBase::cancelAll()
	{
		// Note that this won't free any Frame data allocated for all the canceled commands since
		// frame data will only get cleared at frame start
		mCommandQueue->cancelAll();
	}
}