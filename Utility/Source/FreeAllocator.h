#pragma once

namespace Utility
{
	/** @addtogroup Memory-Internal
	 *  @{
	 */

	 /**
	  * Free allocator with no limitations, using traditional malloc/free under the hood. */
	class FreeAllocator
	{
	public:
		/** Allocates memory. */
		uint8_t* alloc(uint32_t amount)
		{
			return (uint8_t*)malloc(amount);
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
}