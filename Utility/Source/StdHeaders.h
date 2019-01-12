#pragma once

#include "Utility.h"

#ifdef __BORLANDC__
#   define __STD_ALGORITHM
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <cstddef>
#include <cmath>

#include <memory>

// STL containers
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <set>
#include <list>
#include <forward_list>
#include <deque>
#include <queue>
#include <bitset>
#include <array>

#include <unordered_map>
#include <unordered_set>

// STL algorithms & functions
#include <algorithm>
#include <functional>
#include <limits>
#include <iterator>

// C++ Stream stuff
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

extern "C" {
    
#   include <sys/types.h>
#   include <sys/stat.h>
    
}

#if PLATFORM_WINDOWS
#  undef min
#  undef max
#  if defined( __MINGW32__ )
#    include <unistd.h>
#  endif
#endif

#if PLATFORM_LINUX
extern "C" {
    
#   include <unistd.h>
#   include <dlfcn.h>
    
}
#endif

#if PLATFORM_OSX
extern "C" {
    
#   include <unistd.h>
#   include <sys/param.h>
#   include <CoreFoundation/CoreFoundation.h>
    
}
#endif

namespace Utility
{
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
}
