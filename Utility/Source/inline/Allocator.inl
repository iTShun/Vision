#include "../Allocator.h"

namespace Utility
{
    template<class T>
    void* MemoryAllocator<T>::allocate(size_t bytes, const char* file, uint32_t line)
    {
        return ::malloc(bytes);
    }
    
    template<class T>
    void* MemoryAllocator<T>::allocateAligned(size_t bytes, size_t alignment, const char* file, uint32_t line)
    {
#   if COMPILER_MSVC
        return _aligned_malloc(bytes, alignment);
#   else
        void* data = ::malloc(bytes + (alignment - 1) + sizeof(void*));
        if (data == nullptr)
            return nullptr;
        
        char* alignedData = ((char*)data) + sizeof(void*);
        alignedData += (alignment - ((uintptr_t)alignedData) & (alignment - 1)) & (alignment - 1);
        
        ((void**)alignedData)[-1] = data;
        return alignedData;
#   endif
    }
    
    template<class T>
    void MemoryAllocator<T>::free(void* ptr, const char* file, uint32_t line)
    {
        ::free(ptr);
    }
    
    template<class T>
    void MemoryAllocator<T>::freeAligned(void* ptr, const char* file, uint32_t line)
    {
        ::free(((void**)ptr)[-1]);
    }
}
