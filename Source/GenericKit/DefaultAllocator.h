#pragma once

#include "GenericKit/Allocator.h"

#ifndef CONFIG_ALLOCATOR_NATURAL_ALIGNMENT
#	define CONFIG_ALLOCATOR_NATURAL_ALIGNMENT 8
#endif // CONFIG_ALLOCATOR_NATURAL_ALIGNMENT

namespace GenericKit
{
	class DefaultAllocator : public IAllocator
	{
	public:
		DefaultAllocator() {}

		virtual ~DefaultAllocator() {}

		virtual void* Realloc(void* ptr, size_t size, size_t align, const char* file, uint32_t line) override
		{
			if (0 == size)
			{
				if (nullptr != ptr)
				{
					if (CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= align)
					{
						::free(ptr);
						return NULL;
					}

#	if COMPILER_MSVC
					_aligned_free(ptr);
#	else
					GenericKit::FreeAligned(this, ptr, file, line);
#	endif
				}

				return NULL;
			}
			else if (NULL == ptr)
			{
				if (CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= align)
				{
					return ::malloc(size);
				}

#	if COMPILER_MSVC
				return _aligned_malloc(size, align);
#	else
				return GenericKit::AllocAligned(this, size, align, file, line);
#	endif
			}

			if (CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= align)
			{
				return ::realloc(ptr, size);
			}

#	if COMPILER_MSVC
			return _aligned_realloc(ptr, size, align);
#	else
			return GenericKit::ReallocAligned(this, ptr, size, align, file, line);
#	endif
		}
	};
}