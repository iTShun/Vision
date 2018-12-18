#include "LSCoreApplication.h"
#include "Platform/LSPlatform.h"
#include "General/LSMessageHandler.h"
#include "General/LSTime.h"
#include "General/LSDynLibManager.h"
#include "Thread/LSThreadPool.h"
#include "Thread/LSTaskScheduler.h"
#include "CoreThread/LSCoreThread.h"
#include "CoreThread/LSCoreObjectManager.h"

namespace ls
{
	CoreApplication::CoreApplication()
	{
		// Ensure all errors are reported properly
		CrashHandler::startUp();
	}

	CoreApplication::~CoreApplication()
	{
        CoreObjectManager::shutDown(); // Must shut down before DynLibManager to ensure all objects are destroyed before unloading their libraries
        DynLibManager::shutDown();
        Time::shutDown();
        
        CoreThread::shutDown();
		TaskScheduler::shutDown();
		ThreadPool::shutDown();
		MessageHandler::shutDown();

		MemStack::endThread();
		Platform::_shutDown();

		CrashHandler::shutDown();
	}

	void CoreApplication::onStartUp()
	{
		UINT32 numWorkerThreads = LS_THREAD_HARDWARE_CONCURRENCY - 1; // Number of cores while excluding current thread.

		Platform::_startUp();
		MemStack::beginThread();

		MessageHandler::startUp();
		ThreadPool::startUp<TThreadPool<ThreadBansheePolicy>>((numWorkerThreads));
		TaskScheduler::startUp();
		TaskScheduler::instance().removeWorker();
        CoreThread::startUp();
        Time::startUp();
        DynLibManager::startUp();
        CoreObjectManager::startUp();
        
	}
}
