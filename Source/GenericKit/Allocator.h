#pragma once

#include "GenericKit/General.h"

#define KitAlloc(_allocator, _size)							GenericKit::Alloc(_allocator, _size, 0, __FILE__, __LINE__)
#define KitRealloc(_allocator, _ptr, _size)					GenericKit::Realloc(_allocator, _ptr, _size, 0, __FILE__, __LINE__)
#define KitAllocAlign(_allocator, _size, _align)			GenericKit::Alloc(_allocator, _size, _align, __FILE__, __LINE__)
#define KitReallocAlign(_allocator, _ptr, _size, _align)	GenericKit::Realloc(_allocator, _ptr, _size, _align, __FILE__, __LINE__)
#define KitFree(_allocator, _ptr)							GenericKit::Free(_allocator, _ptr, __FILE__, __LINE__)
#define KitDelete(_allocator, _ptr)							GenericKit::DeleteObject(_allocator, _ptr, __FILE__, __LINE__)

#define PLACEMENT_NEW(_ptr, _type)				::new(GenericKit::PlacementNewTag(), _ptr) _type
#define NEW(_allocator, _type)					PLACEMENT_NEW(KitAlloc(_allocator, sizeof(_type) ), _type)
#define NEW_ALIGN(_allocator, _type, _align)	PLACEMENT_NEW(KitAllocAlign(_allocator, sizeof(_type), _align), _type)
#define DELETE(_allocator, _ptr)				KitDelete(_allocator, _ptr)

namespace GenericKit { struct PlacementNewTag {}; }

void* operator new(size_t, GenericKit::PlacementNewTag, void* ptr);
void  operator delete(void*, GenericKit::PlacementNewTag, void*) throw();

namespace GenericKit
{
	/// Abstract allocator interface.
	///
	struct IAllocator
	{
		virtual ~IAllocator() = 0;

		virtual void* Realloc(void* ptr, size_t size, size_t align, const char* file, uint32_t line) = 0;
	};

	/// Aligns pointer to nearest next aligned address. _align must be power of two.
	void* AlignPtr(void* ptr, size_t extra, size_t align = 0);

	/// Allocate memory.
	void* Alloc(IAllocator* allocator, size_t size, size_t align = 0, const char* file = NULL, uint32_t line = 0);

	/// Free memory.
	void Free(IAllocator* allocator, void* ptr, const char* file = NULL, uint32_t line = 0);

	/// Resize memory block.
	void* Realloc(IAllocator* allocator, void* ptr, size_t size, size_t align = 0, const char* file = NULL, uint32_t line = 0);

	/// Allocate memory with specific alignment.
	void* AllocAligned(IAllocator* allocator, size_t size, size_t align = 0, const char* file = NULL, uint32_t line = 0);

	/// Free memory that was allocated with aligned allocator.
	void FreeAligned(IAllocator* allocator, void* ptr, const char* file = NULL, uint32_t line = 0);

	/// Resize memory block that was allocated with aligned allocator.
	void* ReallocAligned(IAllocator* allocator, void* ptr, size_t size, size_t align = 0, const char* file = NULL, uint32_t line = 0);

	/// Delete object with specific allocator.
	template <typename ObjectT>
	void DeleteObject(IAllocator* allocator, ObjectT* object, const char* file = NULL, uint32_t line = 0);
}

#include "GenericKit/Allocator.inl"
