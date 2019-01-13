#pragma once

#include "Config.h"
#include "StdHeaders.h"

#include <new>
#include <limits>
#include <cstdint>
#include <utility>

#if PLATFORM_OSX
#   include <malloc/malloc.h>
#   include <alloca.h>
#elif COMPILER_MSVC
#   include <malloc.h>
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
        static void* allocate(size_t bytes, const char* file = NULL, uint32_t line = 0)
		{
			return ::malloc(bytes);
		}
        
        /** Allocates @p bytes and aligns them to the specified boundary (in bytes). */
        static void* allocateAligned(size_t bytes, size_t alignment, const char* file = NULL, uint32_t line = 0)
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
        
        /** Frees the memory at the specified location. */
        static void free(void* ptr, const char* file = NULL, uint32_t line = 0)
		{
			::free(ptr);
		}
        
        /** Frees memory allocated with allocateAligned() */
        static void freeAligned(void* ptr, const char* file = NULL, uint32_t line = 0)
		{
#   if COMPILER_MSVC
			_aligned_free(ptr);
#	else
			::free(((void**)ptr)[-1]);
#	endif
		}
    };
    
	/**
	 * General allocator provided by the OS. Use for persistent long term allocations, and allocations that don't
	 * happen often.
	 */
	class GenAllocator
	{ };

	/** @addtogroup Memory
	 *  @{
	 */

	/** Allocates the specified number of bytes. */
	template<class Allocator>
	inline void* Alloc(size_t count)
	{
		return MemoryAllocator<Allocator>::allocate(count);
	}

	/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
	template<class T, class Allocator>
	inline T* Alloc()
	{
		return (T*)MemoryAllocator<Allocator>::allocate(sizeof(T));
	}

	/** Creates and constructs an array of @p count elements. */
	template<class T, class Allocator>
	inline T* NewN(size_t count)
	{
		T* ptr = (T*)MemoryAllocator<Allocator>::allocate(sizeof(T) * count);

		for (size_t i = 0; i < count; ++i)
			new (&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template<class Type, class Allocator, class... Args>
	Type* New(Args &&...args)
	{
		return new (Alloc<Type, Allocator>()) Type(std::forward<Args>(args)...);
	}

	/** Frees all the bytes allocated at the specified location. */
	template<class Allocator>
	inline void Free(void* ptr)
	{
		MemoryAllocator<Allocator>::free(ptr);
	}

	/** Destructs and frees the specified object. */
	template<class T, class Allocator = GenAllocator>
	inline void Delete(T* ptr)
	{
		(ptr)->~T();

		MemoryAllocator<Allocator>::free(ptr);
	}

	/** Destructs and frees the specified array of objects. */
	template<class T, class Allocator = GenAllocator>
	inline void DeleteN(T* ptr, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
			ptr[i].~T();

		MemoryAllocator<Allocator>::free(ptr);
	}

	/*****************************************************************************/
	/* Default versions of all alloc/free/new/delete methods which call GenAlloc */
	/*****************************************************************************/

	/** Allocates the specified number of bytes. */
	inline void* Alloc(size_t count)
	{
		return MemoryAllocator<GenAllocator>::allocate(count);
	}

	/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
	template<class T>
	inline T* Alloc()
	{
		return (T*)MemoryAllocator<GenAllocator>::allocate(sizeof(T));
	}

	/**
	 * Allocates the specified number of bytes aligned to the provided boundary. Boundary is in bytes and must be a power
	 * of two.
	 */
	inline void* AllocAligned(size_t count, size_t align)
	{
		return MemoryAllocator<GenAllocator>::allocateAligned(count, align);
	}

	/** Allocates enough bytes to hold an array of @p count elements the specified type, but doesn't construct them. */
	template<class T>
	inline T* AllocN(size_t count)
	{
		return (T*)MemoryAllocator<GenAllocator>::allocate(count * sizeof(T));
	}

	/** Creates and constructs an array of @p count elements. */
	template<class T>
	inline T* NewN(size_t count)
	{
		T* ptr = (T*)MemoryAllocator<GenAllocator>::allocate(count * sizeof(T));

		for (size_t i = 0; i < count; ++i)
			new (&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template<class Type, class... Args>
	Type* New(Args &&...args)
	{
		return new (Alloc<Type, GenAllocator>()) Type(std::forward<Args>(args)...);
	}

	/** Frees all the bytes allocated at the specified location. */
	inline void Free(void* ptr)
	{
		MemoryAllocator<GenAllocator>::free(ptr);
	}

	/** Frees memory previously allocated with bs_alloc_aligned(). */
	inline void FreeAligned(void* ptr)
	{
		MemoryAllocator<GenAllocator>::freeAligned(ptr);
	}

	/************************************************************************/
	/*			MACRO VERSIONS					*/
	/* You will almost always want to use the template versions but in some */
	/* cases (private destructor) it is not possible. In which case you may	*/
	/* use these instead.							*/
	/************************************************************************/
#	define PVT_DELETE(T, ptr) \
		(ptr)->~T(); \
		MemoryAllocator<GenAllocator>::free(ptr);

#	define PVT_DELETE_A(T, ptr, Allocator) \
		(ptr)->~T(); \
		MemoryAllocator<Allocator>::free(ptr);

	 /** @} */

    /** Allocator for the standard library that internally uses Banshee memory allocator. */
    template <class T, class Allocator = GenAllocator>
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
        template<class U, class Allocator2> constexpr StdAlloc(const StdAlloc<U, Allocator2>&) { };
        template<class U, class Allocator2> constexpr bool operator==(const StdAlloc<U, Allocator2>&) const noexcept { return true; }
        template<class U, class Allocator2> constexpr bool operator!=(const StdAlloc<U, Allocator2>&) const noexcept { return false; }
        
        template<class U> class rebind { public: using other = StdAlloc<U, Allocator>; };
        
        /** Allocate but don't initialize number elements of type T. */
        static T* allocate(const size_t num)
        {
            if (num == 0)
                return nullptr;
            
            if (num > std::numeric_limits<size_t>::max() / sizeof(T))
                return nullptr; // Error
            
            void* const pv = Alloc<Allocator>(num * sizeof(T));
            if (!pv)
                return nullptr; // Error
            
            return static_cast<T*>(pv);
        }
        
        /** Deallocate storage p of deleted elements. */
        static void deallocate(pointer p, size_type)
        {
            Free<Allocator>(p);
        }
        
        static constexpr size_t max_size() { return std::numeric_limits<size_type>::max() / sizeof(T); }
        static constexpr void destroy(pointer p) { p->~T(); }
        
        template<class... Args>
        static void construct(pointer p, Args&&... args) { new(p) T(std::forward<Args>(args)...); }
    };
    
    /** @} */
}

namespace Utility
{
    /**
     * Hash for enum types, to be used instead of std::hash<T> when T is an enum.
     *
     * Until C++14, std::hash<T> is not defined if T is a enum (see
     * http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148).  But
     * even with C++14, as of october 2016, std::hash for enums is not widely
     * implemented by compilers, so here when T is a enum, we use EnumClassHash
     * instead of std::hash. (For instance, in Utility::hash_combine(), or
     * Utility::UnorderedMap.)
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
    template <typename T, typename Allocator = GenAllocator, typename Deleter = decltype(&Delete<T, Allocator>)>
    using UPtr = std::unique_ptr<T, Deleter>;
    
    /** Create a new shared pointer using a custom allocator category. */
    template<typename Type, typename AllocatorCategory = GenAllocator, typename... Args>
    SPtr<Type> SharedPtrNew(Args &&... args)
    {
        return std::allocate_shared<Type>(StdAlloc<Type, AllocatorCategory>(), std::forward<Args>(args)...);
    }
    
    /**
     * Create a new shared pointer from a previously constructed object.
     * Pointer specific data will be allocated using the provided allocator category.
     */
    template<typename Type, typename MainAllocator = GenAllocator, typename PtrDataAllocator = GenAllocator, typename Deleter = decltype(&Delete<Type, MainAllocator>)>
    SPtr<Type> SharedPtr(Type* data, Deleter del = &Delete<Type, MainAllocator>)
    {
        return SPtr<Type>(data, std::move(del), StdAlloc<Type, PtrDataAllocator>());
    }
    
    /**
     * Create a new unique pointer from a previously constructed object.
     * Pointer specific data will be allocated using the provided allocator category.
     */
    template<typename Type, typename Allocator = GenAllocator, typename Deleter = decltype(&Delete<Type, Allocator>)>
    UPtr<Type, Allocator, Deleter> UniquePtr(Type* data, Deleter del = &Delete<Type, Allocator>)
    {
        return std::unique_ptr<Type, Deleter>(data, std::move(del));
    }
    
    /** Create a new unique pointer using a custom allocator category. */
    template<typename Type, typename Allocator = GenAllocator, typename Deleter = decltype(&Delete<Type, Allocator>), typename... Args>
    UPtr<Type, Allocator, Deleter> UniquePtrNew(Args &&... args)
    {
        Type* rawPtr = New<Type, Allocator>(std::forward<Args>(args)...);
        
        return UniquePtr<Type, Allocator, Deleter>(rawPtr);
    }
    
    /**
     * "Smart" pointer that is not smart. Does nothing but hold a pointer value. No memory management is performed at all.
     * This class exists to make storing pointers in containers easier to manage, such as with non-member comparison
     * operators.
     */
    template<typename T>
    struct NativePtr
    {
        constexpr NativePtr(T* p) : mPtr(p) {}
        constexpr T& operator*() const { return *mPtr; }
        constexpr T* operator->() const { return mPtr; }
        constexpr T* get() const { return mPtr; }
        
    private:
        T* mPtr = nullptr;
    };
    
    template<typename T>
    using NPtr = NativePtr<T>;
    
    template<typename L_T, typename R_T>
    constexpr bool operator< (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
    {
        return lhs.get() < rhs.get();
    }
    
    template<typename L_T, typename R_T>
    constexpr bool operator> (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
    {
        return lhs.get() > rhs.get();
    }
    
    template<typename L_T, typename R_T>
    constexpr bool operator<= (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
    {
        return lhs.get() <= rhs.get();
    }
    
    template<typename L_T, typename R_T>
    constexpr bool operator>= (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
    {
        return lhs.get() >= rhs.get();
    }
    
    template<typename L_T, typename R_T>
    constexpr bool operator== (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
    {
        return lhs.get() == rhs.get();
    }
    
    template<typename L_T, typename R_T>
    constexpr bool operator!= (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
    {
        return lhs.get() != rhs.get();
    }
    
    /** @} */
}
