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
	 * Free allocator with no limitations, using traditional malloc/free under the hood. */
	class FreeAlloc
	{
	public:
		/** Allocates memory. */
		UINT8* alloc(UINT32 amount)
		{
			return (UINT8*)malloc(amount);
		}

		/** Deallocates a previously allocated piece of memory. */
		void free(void* data)
		{
			::free(data);
		}

		/** Unused */
		void clear()
		{
			// Do nothing
		}
	};

	/** @} */
	/** @} */
}
