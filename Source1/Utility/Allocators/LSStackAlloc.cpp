#include "Prerequisites/LSPrerequisitesUtil.h"
#include "Allocators/LSStackAlloc.h"

namespace ls
{
	LS_THREADLOCAL MemStackInternal<1024 * 1024>* MemStack::ThreadMemStack = nullptr;

	void MemStack::beginThread()
	{
		if(ThreadMemStack != nullptr)
			endThread();

		ThreadMemStack = ls_new<MemStackInternal<1024 * 1024>>();
	}

	void MemStack::endThread()
	{
		if(ThreadMemStack != nullptr)
		{
			ls_delete(ThreadMemStack);
			ThreadMemStack = nullptr;
		}
	}

	UINT8* MemStack::alloc(UINT32 numBytes)
	{
		assert(ThreadMemStack != nullptr && "Stack allocation failed. Did you call beginThread?");

		return ThreadMemStack->alloc(numBytes);
	}

	void MemStack::deallocLast(UINT8* data)
	{
		assert(ThreadMemStack != nullptr && "Stack deallocation failed. Did you call beginThread?");

		ThreadMemStack->dealloc(data);
	}
}
