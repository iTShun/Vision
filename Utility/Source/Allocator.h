#pragma once

#include "Platform.h"

#include <new>
#include <limits>
#include <cstdint>
#include <utility>

#if PLATFORM_OSX
#   include <malloc/malloc.h>
#   include <alloca.h>
#endif

namespace Utility
{
    /** @addtogroup Memory-Internal
     *  @{
     */
    
    /** Memory allocator providing a generic implementation. */
    template<class T>
    class MemoryAllocator
    {
    public:
        /** Allocates @p bytes bytes. */
        static void* allocate(size_t bytes, const char* file = NULL, uint32_t line = 0);
        
        /** Allocates @p bytes and aligns them to the specified boundary (in bytes). */
        static void* allocateAligned(size_t bytes, size_t alignment, const char* file = NULL, uint32_t line = 0);
        
        /** Frees the memory at the specified location. */
        static void free(void* ptr, const char* file = NULL, uint32_t line = 0);
        
        /** Frees memory allocated with allocateAligned() */
        static void freeAligned(void* ptr, const char* file = NULL, uint32_t line = 0);
    };
    
    /**
     * General allocator provided by the OS. Use for persistent long term allocations, and allocations that don't
     * happen often.
     */
    class GenAlloc
    { };
    
    /** Allocator for the standard library that internally uses Banshee memory allocator. */
    template <class T, class Alloc = GenAlloc>
    class StdAlloc
    {
    public:
        using value_type        = T;
        using pointer           = value_type*;
        using const_pointer     = const value_type*;
        using reference         = value_type&;
        using const_reference   = const value_type&;
        using size_type         = std::size_t;
        using difference_type   = std::ptrdiff_t;
        
        constexpr StdAlloc() = default;
        constexpr StdAlloc(StdAlloc&&) = default;
        constexpr StdAlloc(const StdAlloc&) = default;
        template<class U, class Alloc2> constexpr StdAlloc(const StdAlloc<U, Alloc2>&) { };
        template<class U, class Alloc2> constexpr bool operator==(const StdAlloc<U, Alloc2>&) const noexcept { return true; }
        template<class U, class Alloc2> constexpr bool operator!=(const StdAlloc<U, Alloc2>&) const noexcept { return false; }
        
        template<class U> class rebind { public: using other = StdAlloc<U, Alloc>; };
        
        /** Allocate but don't initialize number elements of type T. */
        static T* allocate(const size_t num)
        {
            if (num == 0)
                return nullptr;
            
            if (num > std::numeric_limits<size_t>::max() / sizeof(T))
                return nullptr; // Error
            
            void* const pv = nullptr;//bs_alloc<Alloc>(num * sizeof(T));
            if (!pv)
                return nullptr; // Error
            
            return static_cast<T*>(pv);
        }
        
        /** Deallocate storage p of deleted elements. */
        static void deallocate(pointer p, size_type)
        {
//            bs_free<Alloc>(p);
        }
        
        static constexpr size_t max_size() { return std::numeric_limits<size_type>::max() / sizeof(T); }
        static constexpr void destroy(pointer p) { p->~T(); }
        
        template<class... Args>
        static void construct(pointer p, Args&&... args) { new(p) T(std::forward<Args>(args)...); }
    };
    
    /** @} */
}

#include "inline/Allocator.inl"
