#include "CoreThread/LSCommandQueue.h"
#include "Error/LSException.h"
#include "CoreThread/LSCoreThread.h"
#include "Logger/LSLogger.h"

namespace ls
{
#if DEBUG_MODE
	CommandQueueBase::CommandQueueBase(ThreadId threadId)
		:mMyThreadId(threadId), mMaxDebugIdx(0)
	{
		mAsyncOpSyncData = ls_shared_ptr_new<AsyncOpSyncData>();
		mCommands = ls_new<ls::Queue<QueuedCommand>>();

		{
			Lock lock(CommandQueueBreakpointMutex);

			mCommandQueueIdx = MaxCommandQueueIdx++;
		}
	}
#else
	CommandQueueBase::CommandQueueBase(ThreadId threadId)
		:mMyThreadId(threadId)
	{
		mAsyncOpSyncData = ls_shared_ptr_new<AsyncOpSyncData>();
		mCommands = ls_new<ls::Queue<QueuedCommand>>();
	}
#endif

	CommandQueueBase::~CommandQueueBase()
	{
		if(mCommands != nullptr)
			ls_delete(mCommands);

		while(!mEmptyCommandQueues.empty())
		{
			ls_delete(mEmptyCommandQueues.top());
			mEmptyCommandQueues.pop();
		}
	}

	AsyncOp CommandQueueBase::queueReturn(std::function<void(AsyncOp&)> commandCallback, bool _notifyWhenComplete, UINT32 _callbackId)
	{
#if DEBUG_MODE
		breakIfNeeded(mCommandQueueIdx, mMaxDebugIdx);

		QueuedCommand newCommand(commandCallback, mMaxDebugIdx++, mAsyncOpSyncData, _notifyWhenComplete, _callbackId);
#else
		QueuedCommand newCommand(commandCallback, mAsyncOpSyncData, _notifyWhenComplete, _callbackId);
#endif

		mCommands->push(newCommand);

#if LS_FORCE_SINGLETHREADED_RENDERING
		Queue<QueuedCommand>* commands = flush();
		playback(commands);
#endif

		return newCommand.asyncOp;
	}

	void CommandQueueBase::queue(std::function<void()> commandCallback, bool _notifyWhenComplete, UINT32 _callbackId)
	{
#if DEBUG_MODE
		breakIfNeeded(mCommandQueueIdx, mMaxDebugIdx);

		QueuedCommand newCommand(commandCallback, mMaxDebugIdx++, _notifyWhenComplete, _callbackId);
#else
		QueuedCommand newCommand(commandCallback, _notifyWhenComplete, _callbackId);
#endif

		mCommands->push(newCommand);

#if LS_FORCE_SINGLETHREADED_RENDERING
		Queue<QueuedCommand>* commands = flush();
		playback(commands);
#endif
	}

	ls::Queue<QueuedCommand>* CommandQueueBase::flush()
	{
		ls::Queue<QueuedCommand>* oldCommands = mCommands;

		if(!mEmptyCommandQueues.empty())
		{
			mCommands = mEmptyCommandQueues.top();
			mEmptyCommandQueues.pop();
		}
		else
		{
			mCommands = ls_new<ls::Queue<QueuedCommand>>();
		}

		return oldCommands;
	}

	void CommandQueueBase::playbackWithNotify(ls::Queue<QueuedCommand>* commands, std::function<void(UINT32)> notifyCallback)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(commands == nullptr)
			return;

		while(!commands->empty())
		{
			QueuedCommand& command = commands->front();

			if(command.returnsValue)
			{
				AsyncOp& op = command.asyncOp;
				command.callbackWithReturnValue(op);

				if(!command.asyncOp.hasCompleted())
				{
					LOGDBG("Async operation return value wasn't resolved properly. Resolving automatically to nullptr. " \
						"Make sure to complete the operation before returning from the command callback method.");
					command.asyncOp._completeOperation(nullptr);
				}
			}
			else
			{
				command.callback();
			}

			if(command.notifyWhenComplete && notifyCallback != nullptr)
			{
				notifyCallback(command.callbackId);
			}

			commands->pop();
		}

		mEmptyCommandQueues.push(commands);
	}

	void CommandQueueBase::playback(ls::Queue<QueuedCommand>* commands)
	{
		playbackWithNotify(commands, std::function<void(UINT32)>());
	}

	void CommandQueueBase::cancelAll()
	{
		ls::Queue<QueuedCommand>* commands = flush();

		while(!commands->empty())
			commands->pop();

		mEmptyCommandQueues.push(commands);
	}

	bool CommandQueueBase::isEmpty()
	{
		if(mCommands != nullptr && mCommands->size() > 0)
			return false;

		return true;
	}

	void CommandQueueBase::throwInvalidThreadException(const String& message) const
	{
		LS_EXCEPT(InternalErrorException, message);
	}

#if DEBUG_MODE
	Mutex CommandQueueBase::CommandQueueBreakpointMutex;

	UINT32 CommandQueueBase::MaxCommandQueueIdx = 0;

	UnorderedSet<CommandQueueBase::QueueBreakpoint, CommandQueueBase::QueueBreakpoint::HashFunction, 
		CommandQueueBase::QueueBreakpoint::EqualFunction> CommandQueueBase::SetBreakpoints;

	inline size_t CommandQueueBase::QueueBreakpoint::HashFunction::operator()(const QueueBreakpoint& v) const
	{
		size_t seed = 0;
		hash_combine(seed, v.queueIdx);
		hash_combine(seed, v.commandIdx);
		return seed;
	}

	inline bool CommandQueueBase::QueueBreakpoint::EqualFunction::operator()(const QueueBreakpoint &a, const QueueBreakpoint &b) const
	{
		return a.queueIdx == b.queueIdx && a.commandIdx == b.commandIdx;
	}

	void CommandQueueBase::addBreakpoint(UINT32 queueIdx, UINT32 commandIdx)
	{
		Lock lock(CommandQueueBreakpointMutex);

		SetBreakpoints.insert(QueueBreakpoint(queueIdx, commandIdx));
	}

	void CommandQueueBase::breakIfNeeded(UINT32 queueIdx, UINT32 commandIdx)
	{
		// I purposely don't use a mutex here, as this gets called very often. Generally breakpoints
		// will only be added at the start of the application, so race conditions should not occur.
		auto iterFind = SetBreakpoints.find(QueueBreakpoint(queueIdx, commandIdx));

		if(iterFind != SetBreakpoints.end())
		{
			assert(false && "Command queue breakpoint triggered!");
		}
	}
#else
	void CommandQueueBase::addBreakpoint(UINT32 queueIdx, UINT32 commandIdx)
	{
		// Do nothing, no breakpoints in release
	}
#endif
}