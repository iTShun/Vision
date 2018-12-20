#pragma once

#include "Platform/LSPlatformMacros.h"
#include "Platform/LSStdHeaders.h"

#undef min
#undef max

#include <new>
#include <limits>
#include <cstdint>
#include <utility>
#include <atomic>

#if PLATFORM_LINUX
#  include <malloc.h>
#endif

namespace ls
{
    class MemoryAllocatorBase;
    
    /** @addtogroup Internal-Utility
     *  @{
     */
    
    /** @addtogroup Memory-Internal
     *  @{
     */
    
    inline void* platformAlignedAlloc(size_t size, size_t alignment)
    {
        void* data = ::malloc(size + (alignment - 1) + sizeof(void*));
        if (data == nullptr)
            return nullptr;
        
        char* alignedData = ((char*)data) + sizeof(void*);
        alignedData += (alignment - ((uintptr_t)alignedData) & (alignment - 1)) & (alignment - 1);
        
        ((void**)alignedData)[-1] = data;
        return alignedData;
    }
    
    inline void platformAlignedFree(void* ptr)
    {
        // TODO: Document how this works.
        ::free(((void**)ptr)[-1]);
    }
    
    /**
     * Thread safe class used for storing total number of memory allocations and deallocations, primarily for statistic
     * purposes.
     */
    class MemoryCounter
    {
    public:
        static uint64_t getNumAllocs()
        {
            return Allocs.load();
        }
        
    private:
        friend class MemoryAllocatorBase;
        
        // Threadlocal data can't be exported, so some magic to make it accessible from MemoryAllocator
        static void incAllocCount() { ++Allocs; }
        static void decAllocCount() { --Allocs; }
        
        static std::atomic<uint64_t> Allocs;
    };
    
    /** Base class all memory allocators need to inherit. Provides allocation and free counting. */
    class MemoryAllocatorBase
    {
    protected:
        static void incAllocCount() { MemoryCounter::incAllocCount(); }
        static void decAllocCount() { MemoryCounter::decAllocCount(); }
    };
    
    /**
     * Memory allocator providing a generic implementation. Specialize for specific categories as needed.
     *
     * @note    For example you might implement a pool allocator for specific types in order
     *             to reduce allocation overhead. By default standard malloc/free are used.
     */
    template<class T>
    class MemoryAllocator : public MemoryAllocatorBase
    {
    public:
        /** Allocates @p bytes bytes. */
        static void* allocate(size_t bytes)
        {
#if PROFILING_ENABLED
            incAllocCount();
#endif
            
            return malloc(bytes);
        }
        
        /**
         * Allocates @p bytes and aligns them to the specified boundary (in bytes). If the aligment is less or equal to
         * 16 it is more efficient to use the allocateAligned16() alternative of this method. Alignment must be power of two.
         */
        static void* allocateAligned(size_t bytes, size_t alignment)
        {
#if PROFILING_ENABLED
            incAllocCount();
#endif
            
            return platformAlignedAlloc(bytes, alignment);
        }
        
        /** Allocates @p bytes and aligns them to a 16 byte boundary. */
        static void* allocateAligned16(size_t bytes)
        {
#if PROFILING_ENABLED
            incAllocCount();
#endif
            
            return platformAlignedAlloc(bytes, 16);
        }
        
        /** Frees the memory at the specified location. */
        static void free(void* ptr)
        {
#if PROFILING_ENABLED
            decAllocCount();
#endif
            
            ::free(ptr);
        }
        
        /** Frees memory allocated with allocateAligned() */
        static void freeAligned(void* ptr)
        {
#if PROFILING_ENABLED
            decAllocCount();
#endif
            
            platformAlignedFree(ptr);
        }
        
        /** Frees memory allocated with allocateAligned16() */
        static void freeAligned16(void* ptr)
        {
#if PROFILING_ENABLED
            decAllocCount();
#endif
            
            platformAlignedFree(ptr);
        }
    };
    
    /**
     * General allocator provided by the OS. Use for persistent long term allocations, and allocations that don't
     * happen often.
     */
    class GenAlloc
    { };
    
    /** @} */
    /** @} */
    
    /** @addtogroup Memory
     *  @{
     */
    
    /** Allocates the specified number of bytes. */
    template<class Alloc>
    inline void* ls_alloc(size_t count)
    {
        return MemoryAllocator<Alloc>::allocate(count);
    }
    
    /** Allocates enough bytes to hold the specified type, but doesn't construct it. */
    template<class T, class Alloc>
    inline T* ls_alloc()
    {
        return (T*)MemoryAllocator<Alloc>::allocate(sizeof(T));
    }
    
    /** Creates and constructs an array of @p count elements. */
    template<class T, class Alloc>
    inline T* ls_newN(size_t count)
    {
        T* ptr = (T*)MemoryAllocator<Alloc>::allocate(sizeof(T) * count);
        
        for(size_t i = 0; i < count; ++i)
            new (&ptr[i]) T;
        
        return ptr;
    }
    
    /** Create a new object with the specified allocator and the specified parameters. */
    template<class Type, class Alloc, class... Args>
    Type* ls_new(Args &&...args)
    {
        return new (ls_alloc<Type, Alloc>()) Type(std::forward<Args>(args)...);
    }
    
    /** Frees all the bytes allocated at the specified location. */
    template<class Alloc>
    inline void ls_free(void* ptr)
    {
        MemoryAllocator<Alloc>::free(ptr);
    }
    
    /** Destructs and frees the specified object. */
    template<class T, class Alloc = GenAlloc>
    inline void ls_delete(T* ptr)
    {
        (ptr)->~T();
        
        MemoryAllocator<Alloc>::free(ptr);
    }
    
    /** Destructs and frees the specified array of objects. */
    template<class T, class Alloc = GenAlloc>
    inline void ls_deleteN(T* ptr, size_t count)
    {
        for(size_t i = 0; i < count; ++i)
            ptr[i].~T();
        
        MemoryAllocator<Alloc>::free(ptr);
    }
    
    /*****************************************************************************/
    /* Default versions of all alloc/free/new/delete methods which call GenAlloc */
    /*****************************************************************************/
    
    /** Allocates the specified number of bytes. */
    inline void* ls_alloc(size_t count)
    {
        return MemoryAllocator<GenAlloc>::allocate(count);
    }
    
    /** Allocates enough bytes to hold the specified type, but doesn't construct it. */
    template<class T>
    inline T* ls_alloc()
    {
        return (T*)MemoryAllocator<GenAlloc>::allocate(sizeof(T));
    }
    
    /**
     * Allocates the specified number of bytes aligned to the provided boundary. Boundary is in bytes and must be a power
     * of two.
     */
    inline void* ls_alloc_aligned(size_t count, size_t align)
    {
        return MemoryAllocator<GenAlloc>::allocateAligned(count, align);
    }
    
    
    /** Allocates the specified number of bytes aligned to a 16 bytes boundary. */
    inline void* ls_alloc_aligned16(size_t count)
    {
        return MemoryAllocator<GenAlloc>::allocateAligned16(count);
    }
    
    /** Allocates enough bytes to hold an array of @p count elements the specified type, but doesn't construct them. */
    template<class T>
    inline T* ls_allocN(size_t count)
    {
        return (T*)MemoryAllocator<GenAlloc>::allocate(count * sizeof(T));
    }
    
    /** Creates and constructs an array of @p count elements. */
    template<class T>
    inline T* ls_newN(size_t count)
    {
        T* ptr = (T*)MemoryAllocator<GenAlloc>::allocate(count * sizeof(T));
        
        for(size_t i = 0; i < count; ++i)
            new (&ptr[i]) T;
        
        return ptr;
    }
    
    /** Create a new object with the specified allocator and the specified parameters. */
    template<class Type, class... Args>
    Type* ls_new(Args &&...args)
    {
        return new (ls_alloc<Type, GenAlloc>()) Type(std::forward<Args>(args)...);
    }
    
    /** Frees all the bytes allocated at the specified location. */
    inline void ls_free(void* ptr)
    {
        MemoryAllocator<GenAlloc>::free(ptr);
    }
    
    /** Frees memory previously allocated with ls_alloc_aligned(). */
    inline void ls_free_aligned(void* ptr)
    {
        MemoryAllocator<GenAlloc>::freeAligned(ptr);
    }
    
    /** Frees memory previously allocated with ls_alloc_aligned16(). */
    inline void ls_free_aligned16(void* ptr)
    {
        MemoryAllocator<GenAlloc>::freeAligned16(ptr);
    }
    
    /************************************************************************/
    /*            MACRO VERSIONS                    */
    /* You will almost always want to use the template versions but in some */
    /* cases (private destructor) it is not possible. In which case you may    */
    /* use these instead.                            */
    /************************************************************************/
#define PVT_DELETE(T, ptr) \
            (ptr)->~T(); \
            MemoryAllocator<GenAlloc>::free(ptr);
    
#define PVT_DELETE_A(T, ptr, Alloc) \
            (ptr)->~T(); \
            MemoryAllocator<Alloc>::free(ptr);
    
    /** @} */
    /** @addtogroup Internal-Utility
     *  @{
     */
    
    /** @addtogroup Memory-Internal
     *  @{
     */
    
    /** Allocator for the standard library that internally uses LSEngine memory allocator. */
    template <class T, class Alloc = GenAlloc>
    class StdAlloc
    {
    public:
        using value_type = T;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        
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
            
            void* const pv = ls_alloc<Alloc>(num * sizeof(T));
            if (!pv)
                return nullptr; // Error
            
            return static_cast<T*>(pv);
        }
        
        /** Deallocate storage p of deleted elements. */
        static void deallocate(pointer p, size_type)
        {
            ls_free<Alloc>(p);
        }
        
        static constexpr size_t max_size() { return std::numeric_limits<size_type>::max() / sizeof(T); }
        static constexpr void destroy(pointer p) { p->~T(); }
        
        template<class... Args>
        static void construct(pointer p, Args&&... args) { new(p) T(std::forward<Args>(args)...); }
    };
    
    /** @} */
    /** @} */
}

namespace ls
{
    /**
     * Hash for enum types, to be used instead of std::hash<T> when T is an enum.
     *
     * Until C++14, std::hash<T> is not defined if T is a enum (see
     * http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148).  But
     * even with C++14, as of october 2016, std::hash for enums is not widely
     * implemented by compilers, so here when T is a enum, we use EnumClassHash
     * instead of std::hash. (For instance, in ls::hash_combine(), or
     * ls::UnorderedMap.)
     */
    struct EnumClassHash
    {
        template <typename T>
        constexpr std::size_t operator()(T t) const
        {
            return static_cast<std::size_t>(t);
        }
    };
    
    /** @addtogroup Containers
     *  @{
     */
    
    /** Hasher that handles custom enums automatically. */
    template <typename Key>
    using HashType = typename std::conditional<std::is_enum<Key>::value, EnumClassHash, std::hash<Key>>::type;
    
    /** Double ended queue. Allows for fast insertion and removal at both its beggining and end. */
    template <typename T, typename A = StdAlloc<T>>
    using Deque = std::deque<T, A>;
    
    /** Dynamically sized array that stores element contigously. */
    template <typename T, typename A = StdAlloc<T>>
    using Vector = std::vector<T, A>;
    
    /**
     * Container that supports constant time insertion and removal for elements with known locations, but without fast
     * random access to elements. Internally implemented as a doubly linked list. Use ForwardList if you do not need
     * reverse iteration.
     */
    template <typename T, typename A = StdAlloc<T>>
    using List = std::list<T, A>;
    
    /**
     * Container that supports constant time insertion and removal for elements with known locations, but without fast
     * random access to elements. Internally implemented as a singly linked list that doesn't support reverse iteration.
     */
    template <typename T, typename A = StdAlloc<T>>
    using ForwardList = std::forward_list<T, A>;
    
    /** First-in, last-out data structure. */
    template <typename T, typename A = StdAlloc<T>>
    using Stack = std::stack<T, std::deque<T, A>>;
    
    /** First-in, first-out data structure. */
    template <typename T, typename A = StdAlloc<T>>
    using Queue = std::queue<T, std::deque<T, A>>;
    
    /** An associative container containing an ordered set of elements. */
    template <typename T, typename P = std::less<T>, typename A = StdAlloc<T>>
    using Set = std::set<T, P, A>;
    
    /** An associative container containing an ordered set of key-value pairs. */
    template <typename K, typename V, typename P = std::less<K>, typename A = StdAlloc<std::pair<const K, V>>>
    using Map = std::map<K, V, P, A>;
    
    /** An associative container containing an ordered set of elements where multiple elements can have the same key. */
    template <typename T, typename P = std::less<T>, typename A = StdAlloc<T>>
    using MultiSet = std::multiset<T, P, A>;
    
    /** An associative container containing an ordered set of key-value pairs where multiple elements can have the same key. */
    template <typename K, typename V, typename P = std::less<K>, typename A = StdAlloc<std::pair<const K, V>>>
    using MultiMap = std::multimap<K, V, P, A>;
    
    /** An associative container containing an unordered set of elements. Usually faster than Set for larger data sets. */
    template <typename T, typename H = HashType<T>, typename C = std::equal_to<T>, typename A = StdAlloc<T>>
    using UnorderedSet = std::unordered_set<T, H, C, A>;
    
    /** An associative container containing an ordered set of key-value pairs. Usually faster than Map for larger data sets. */
    template <typename K, typename V, typename H = HashType<K>, typename C = std::equal_to<K>, typename A = StdAlloc<std::pair<const K, V>>>
    using UnorderedMap = std::unordered_map<K, V, H, C, A>;
    
    /**
     * An associative container containing an ordered set of key-value pairs where multiple elements can have the same key.
     * Usually faster than MultiMap for larger data sets.
     */
    template <typename K, typename V, typename H = HashType<K>, typename C = std::equal_to<K>, typename A = StdAlloc<std::pair<const K, V>>>
    using UnorderedMultimap = std::unordered_multimap<K, V, H, C, A>;
    
    /** @} */
    
    /** @addtogroup Memory
     *  @{
     */
    
    /**
     * Smart pointer that retains shared ownership of an project through a pointer. The object is destroyed automatically
     * when the last shared pointer to the object is destroyed.
     */
    template <typename T>
    using SPtr = std::shared_ptr<T>;
    
    /**
     * Smart pointer that retains shared ownership of an project through a pointer. Reference to the object must be unique.
     * The object is destroyed automatically when the pointer to the object is destroyed.
     */
    template <typename T, typename Alloc = GenAlloc, typename Delete = decltype(&ls_delete<T, Alloc>)>
    using UPtr = std::unique_ptr<T, Delete>;
    
    /** Create a new shared pointer using a custom allocator category. */
    template<typename Type, typename AllocCategory = GenAlloc, typename... Args>
    SPtr<Type> ls_shared_ptr_new(Args &&... args)
    {
        return std::allocate_shared<Type>(StdAlloc<Type, AllocCategory>(), std::forward<Args>(args)...);
    }
    
    /**
     * Create a new shared pointer from a previously constructed object.
     * Pointer specific data will be allocated using the provided allocator category.
     */
    template<typename Type, typename MainAlloc = GenAlloc, typename PtrDataAlloc = GenAlloc, typename Deleter = decltype(&ls_delete<Type, MainAlloc>)>
    SPtr<Type> ls_shared_ptr(Type* data, Deleter del = &ls_delete<Type, MainAlloc>)
    {
        return SPtr<Type>(data, std::move(del), StdAlloc<Type, PtrDataAlloc>());
    }
    
    /**
     * Create a new unique pointer from a previously constructed object.
     * Pointer specific data will be allocated using the provided allocator category.
     */
    template<typename Type, typename Alloc = GenAlloc, typename Deleter = decltype(&ls_delete<Type, Alloc>)>
    UPtr<Type, Alloc, Deleter> ls_unique_ptr(Type* data, Deleter del = &ls_delete<Type, Alloc>)
    {
        return std::unique_ptr<Type, Deleter>(data, std::move(del));
    }
    
    /** Create a new unique pointer using a custom allocator category. */
    template<typename Type, typename Alloc = GenAlloc, typename Deleter = decltype(&ls_delete<Type, Alloc>), typename... Args>
    UPtr<Type, Alloc, Deleter> ls_unique_ptr_new(Args &&... args)
    {
        Type* rawPtr = ls_new<Type, Alloc>(std::forward<Args>(args)...);
        
        return ls_unique_ptr<Type, Alloc, Deleter>(rawPtr);
    }
    
    /** @} */
}
