#pragma once

#include "GenericKit/MemPool.h"

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace GenericKit
{
    static MemPool g_DefaultPool(1*1024*1024);

    template <class T>
    class DefaultAllocator
    {
    public:
        typedef T                 value_type;
        typedef value_type*       pointer;
        typedef const value_type* const_pointer;
        typedef value_type&       reference;
        typedef const value_type& const_reference;
        typedef std::size_t       size_type;
        typedef std::ptrdiff_t    difference_type;

        DefaultAllocator() = default;
        DefaultAllocator(const DefaultAllocator&) = default;
        ~DefaultAllocator() = default;

        template <class U>
        DefaultAllocator(const DefaultAllocator<U>&) {}

        template <class U>
        struct rebind { typedef DefaultAllocator<U> other; };

        pointer address(reference x) const { return &x; }
        const_pointer address(const_reference x) const { return &x; }

        pointer allocate(size_type n, const_pointer = 0) {
            void* p = g_DefaultPool.alloc(n * sizeof(T));
            if (!p)
                throw std::bad_alloc();
            return static_cast<pointer>(p);
        }

        void deallocate(pointer p, size_type) {
            g_DefaultPool.free(p);
        }

        size_type max_size() const {
            return static_cast<size_type>(-1) / sizeof(value_type);
        }

        void construct(pointer p, const value_type& x) {
            new(p) value_type(x);
        }

        void destroy(pointer p) { p->~value_type(); }

    private:
        void operator=(const DefaultAllocator&);
    };

    template<> class DefaultAllocator<void>
    {
        typedef void        value_type;
        typedef void*       pointer;
        typedef const void* const_pointer;

        template <class U>
        struct rebind { typedef DefaultAllocator<U> other; };
    };

    template <class T>
    inline bool operator==(const DefaultAllocator<T>&,
        const DefaultAllocator<T>&) {
        return true;
    }

    template <class T>
    inline bool operator!=(const DefaultAllocator<T>&,
        const DefaultAllocator<T>&) {
        return false;
    }

    template <typename T>
    using Vector = std::vector<T, DefaultAllocator<T>>;

    template <typename T>
    using Deque = std::deque<T, DefaultAllocator<T>>;

    template <typename T>
    using List = std::list<T, DefaultAllocator<T>>;

    template <typename T>
    using ForwardList = std::forward_list<T, DefaultAllocator<T>>;

    template <typename T>
    using Set = std::set<T, std::less<T>, DefaultAllocator<T>>;

    template <typename T>
    using MultiSet = std::multiset<T, std::less<T>, DefaultAllocator<T>>;

    template <typename Key, typename Value>
    using Map = std::map<Key, Value, std::less<Key>, DefaultAllocator<Value>>;

    template <typename Key, typename Value>
    using MultiMap = std::multimap<Key, Value, std::less<Key>, DefaultAllocator<Value>>;

    template <typename T>
    using UnorderedSet = std::unordered_set<T, std::hash<T>, std::equal_to<T>, DefaultAllocator<T>>;

    template <typename T>
    using UnorderedMultiSet = std::unordered_multiset<T, std::hash<T>, std::equal_to<T>, DefaultAllocator<T>>;

    template <typename Key, typename Value>
    using UnorderedMap = std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, DefaultAllocator<Value>>;

    template <typename Key, typename Value>
    using UnorderedMultiMap = std::unordered_multimap<Key, Value, std::hash<Key>, std::equal_to<Key>, DefaultAllocator<Value>>;

    template <typename T>
    using Stack = std::stack<T, Deque<T>>;

    template <typename T>
    using Queue = std::queue<T, Deque<T>>;

    template <typename T>
    using PriorityQueue = std::priority_queue<T, Deque<T>>;
}

