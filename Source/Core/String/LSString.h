#pragma once

#include "Platform/LSPlatform.h"
#include <string>

namespace ls
{
    /** @addtogroup String
     *  @{
     */
    
    /** Basic string that uses Banshee memory allocators. */
    template <typename T>
    using BasicString = std::basic_string < T, std::char_traits<T>, StdAlloc<T> > ;
    
    /**    Basic string stream that uses Banshee memory allocators. */
    template <typename T>
    using BasicStringStream = std::basic_stringstream < T, std::char_traits<T>, StdAlloc<T> > ;
    
    /** Wide string used primarily for handling Unicode text (UTF-32 on Linux, UTF-16 on Windows, generally). */
    using WString = BasicString<wchar_t>;
    
    /** Narrow string used for handling narrow encoded text (either locale specific ANSI or UTF-8). */
    using String = BasicString<char>;
    
    /** Wide string used UTF-16 encoded strings. */
    using U16String = BasicString<char16_t>;
    
    /** Wide string used UTF-32 encoded strings. */
    using U32String = BasicString<char32_t>;
    
    /** Wide string stream used for primarily for constructing wide strings. */
    using WStringStream =  BasicStringStream<wchar_t>;
    
    /** Wide string stream used for primarily for constructing narrow strings. */
    using StringStream = BasicStringStream<char>;
    
    /** Wide string stream used for primarily for constructing UTF-16 strings. */
    using U16StringStream = BasicStringStream<char16_t>;
    
    /** Wide string stream used for primarily for constructing UTF-32 strings. */
    using U32StringStream = BasicStringStream<char32_t>;
    
    /** Equivalent to String, except it avoids any dynamic allocations until the number of elements exceeds @p Count. */
    template <int Count>
    using SmallString = std::basic_string <char, std::char_traits<char>, StdAlloc<char>>; // TODO: Currently equivalent to String, need to implement the allocator
    
    /** @} */
}

#include "String/LSStringFormat.h"
