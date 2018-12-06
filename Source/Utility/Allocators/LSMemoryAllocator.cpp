#include "Prerequisites/LSPrerequisitesUtil.h"

namespace ls
{
	UINT64 LS_THREADLOCAL MemoryCounter::Allocs = 0;
	UINT64 LS_THREADLOCAL MemoryCounter::Frees = 0;

	void MemoryCounter::incAllocCount() { ++Allocs; }
	void MemoryCounter::incFreeCount() { ++Frees; }
}