#include "LSMemoryAllocator.h"

namespace ls
{
    std::atomic<uint64_t> MemoryCounter::Allocs(0);
}
