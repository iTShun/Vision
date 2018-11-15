/////////////////////////////////////////////////////////////////////
#include "GenericKit/Allocator.h"

inline void* operator new(size_t, GenericKit::PlacementNewTag, void* ptr)
{
    return ptr;
}

inline void operator delete(void*, GenericKit::PlacementNewTag, void*) throw()
{
}

namespace GenericKit
{
    inline IAllocator::~IAllocator()
    {
    }

    inline bool IsAligned(const void* _ptr, size_t _align)
    {
        union { const void* ptr; uintptr_t addr; } un;
        un.ptr = _ptr;
        return 0 == (un.addr & (_align - 1));
    }

    inline void* AlignPtr(void* _ptr, size_t _extra, size_t _align)
    {
        union { void* ptr; uintptr_t addr; } un;
        un.ptr = _ptr;
        uintptr_t unaligned = un.addr + _extra; // space for header
        uintptr_t mask = _align - 1;
        uintptr_t aligned = ALIGN_MASK(unaligned, mask);
        un.addr = aligned;
        return un.ptr;
    }

    /// Allocate memory.
    inline void* Alloc(IAllocator* allocator, size_t size, size_t align, const char* file, uint32_t line)
    {
        return allocator->Realloc(NULL, size, align, file, line);
    }

    /// Free memory.
    inline void Free(IAllocator* allocator, void* ptr, const char* file, uint32_t line)
    {
        allocator->Realloc(ptr, 0, 0, file, line);
    }

    /// Resize memory block.
    inline void* Realloc(IAllocator* allocator, void* ptr, size_t size, size_t align, const char* file, uint32_t line)
    {
        return allocator->Realloc(ptr, size, align, file, line);
    }

    inline void* AllocAligned(IAllocator* _allocator, size_t _size, size_t _align, const char* _file, uint32_t _line)
    {
        const size_t align = Max(_align, sizeof(uint32_t));
        const size_t total = _size + align;
        uint8_t* ptr = (uint8_t*)Alloc(_allocator, total, 0, _file, _line);
        uint8_t* aligned = (uint8_t*)AlignPtr(ptr, sizeof(uint32_t), align);
        uint32_t* header = (uint32_t*)aligned - 1;
        *header = uint32_t(aligned - ptr);
        return aligned;
    }

    inline void FreeAligned(IAllocator* _allocator, void* _ptr, const char* _file, uint32_t _line)
    {
        uint8_t* aligned = (uint8_t*)_ptr;
        uint32_t* header = (uint32_t*)aligned - 1;
        uint8_t* ptr = aligned - *header;
        Free(_allocator, ptr, _file, _line);
    }

    inline void* ReallocAligned(IAllocator* _allocator, void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line)
    {
        if (nullptr == _ptr)
        {
            return AllocAligned(_allocator, _size, _align, _file, _line);
        }

        uint8_t* aligned = (uint8_t*)_ptr;
        uint32_t offset = *((uint32_t*)aligned - 1);
        uint8_t* ptr = aligned - offset;

        const size_t align = Max(_align, sizeof(uint32_t));;
        const size_t total = _size + align;
        ptr = (uint8_t*)Realloc(_allocator, ptr, total, 0, _file, _line);
        uint8_t* newAligned = (uint8_t*)AlignPtr(ptr, sizeof(uint32_t), align);

        if (newAligned == aligned)
        {
            return aligned;
        }

        aligned = ptr + offset;
        MemMove(newAligned, aligned, _size);
        uint32_t* header = (uint32_t*)newAligned - 1;
        *header = uint32_t(newAligned - ptr);
        return newAligned;
    }

    /// Delete object with specific allocator.
    template <typename ObjectT>
    inline void DeleteObject(IAllocator* allocator, ObjectT* object, const char* file, uint32_t line)
    {
        if (nullptr != object)
        {
            object->~ObjectT();
            Free(allocator, object, file, line);
        }
    }
}

