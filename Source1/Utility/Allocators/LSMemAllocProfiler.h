#pragma once

namespace ls
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Specialized allocator for profiler so we can avoid tracking internal profiler memory allocations which would skew 
	 * profiler results.
	 */
	class ProfilerAlloc
	{};

	/** Memory allocator providing a generic implementation. Specialize for specific categories as needed. */
	template<>
	class MemoryAllocator<ProfilerAlloc> : public MemoryAllocatorBase
	{
	public:
		/** Allocates the given number of bytes. */
		static void* allocate(size_t bytes)
		{
			return malloc(bytes);
		}

		/** Frees memory previously allocated with allocate(). */
		static void free(void* ptr)
		{
			::free(ptr);
		}
	};

	/** @} */
	/** @} */
}