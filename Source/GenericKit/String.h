#pragma once

#include "GenericKit/StringUtils.h"
#include "GenericKit/Allocator.h"
#include <string.h>
#include <ctype.h>
#include <vector>

namespace GenericKit
{
    static const int CONVERSION_BUFFER_LENGTH = 128;
    static const int MATRIX_CONVERSION_BUFFER_LENGTH = 256;
    
    class WString;
    
    class String
    {
    protected:
        /// String allocator.
        IAllocator* Allocator;
        /// String length.
        unsigned Length;
        /// Capacity, zero if buffer not allocated.
        unsigned Capacity;
        /// String buffer, null if not allocated.
        char* Buffer;
        
        /// End zero for empty strings.
        static char EndZero;
    public:
        /// Position for "not found."
        static const unsigned NPOS = 0xffffffff;
        /// Initial dynamic allocation size.
        static const unsigned MIN_CAPACITY = 8;
        /// Empty string.
        static const String EMPTY;
        
    public:
        String();

        String(const String& str);
        
        String(const char* str);
        
        String(char* str);
        
        String(const char* str, unsigned length);
        
        String(const wchar_t* str);
        
        String(wchar_t* str);
        
        String(const WString& str);
        
        explicit String(int value);
        
        explicit String(short value);
        
        explicit String(long value);
        
        explicit String(long long value);
        
        explicit String(unsigned value);
        
        explicit String(unsigned short value);
        
        explicit String(unsigned long value);
        
        explicit String(unsigned long long value);
        
        explicit String(float value);
        
        explicit String(double value);
        
        explicit String(bool value);
        
        explicit String(char value);
        
        explicit String(char value, unsigned length);
        
        template <class T>
        explicit String(const T& value);
        
        ~String();
        
        String& operator =(const String& rhs);
        String& operator =(const char* rhs);
        String& operator +=(const String& rhs);
        String& operator +=(const char* rhs);
        String& operator +=(char rhs);
        String& operator +=(int rhs);
        String& operator +=(short rhs);
        String& operator +=(long rhs);
        String& operator +=(long long rhs);
        String& operator +=(unsigned rhs);
        String& operator +=(unsigned short rhs);
        String& operator +=(unsigned long rhs);
        String& operator +=(unsigned long long rhs);
        String& operator +=(float rhs);
        String& operator +=(bool rhs);
        template <class T> String operator +=(const T& rhs);
        String operator +(const String& rhs) const;
        String operator +(const char* rhs) const;
        bool operator ==(const String& rhs) const;
        bool operator !=(const String& rhs) const;
        bool operator <(const String& rhs) const;
        bool operator >(const String& rhs) const;
        bool operator ==(const char* rhs) const;
        bool operator !=(const char* rhs) const;
        bool operator <(const char* rhs) const;
        bool operator >(const char* rhs) const;
        char& operator [](unsigned index);
        const char& operator [](unsigned index) const;
        char& At(unsigned index);
        const char& At(unsigned index) const;
        
        /// Replace all occurrences of a character.
        void Replace(char replaceThis, char replaceWith, bool caseSensitive = true);
        /// Replace all occurrences of a string.
        void Replace(const String& replaceThis, const String& replaceWith, bool caseSensitive = true);
        /// Replace a substring.
        void Replace(unsigned pos, unsigned length, const String& replaceWith);
        /// Replace a substring with a C string.
        void Replace(unsigned pos, unsigned length, const char* replaceWith);
        /// Return a string with all occurrences of a character replaced.
        String Replaced(char replaceThis, char replaceWith, bool caseSensitive = true) const;
        /// Return a string with all occurrences of a string replaced.
        String Replaced(const String& replaceThis, const String& replaceWith, bool caseSensitive = true) const;
        /// Append a string.
        String& Append(const String& str);
        /// Append a C string.
        String& Append(const char* str);
        /// Append a character.
        String& Append(char c);
        /// Append characters.
        String& Append(const char* str, unsigned length);
        /// Insert a string.
        void Insert(unsigned pos, const String& str);
        /// Insert a character.
        void Insert(unsigned pos, char c);
        /// Erase a substring.
        void Erase(unsigned pos, unsigned length = 1);
        /// Resize the string.
        void Resize(unsigned newLength, IAllocator* allocator = nullptr);
        /// Set new capacity.
        void Reserve(unsigned newCapacity);
        /// Reallocate so that no extra memory is used.
        void Compact();
        /// Clear the string.
        void Clear();
        /// Swap with another string.
        void Swap(String& str);
        
        /// Return first char, or 0 if empty.
        char Front() const;
        
        /// Return last char, or 0 if empty.
        char Back() const;
        
        /// Return a substring from position to end.
        String Substring(unsigned pos) const;
        /// Return a substring with length from position.
        String Substring(unsigned pos, unsigned length) const;
        /// Return string with whitespace trimmed from the beginning and the end.
        String Trimmed() const;
        /// Return string in uppercase.
        String ToUpper() const;
        /// Return string in lowercase.
        String ToLower() const;
        /// Return substrings split by a separator char. By default don't return empty strings.
        std::vector<String> Split(char separator, bool keepEmptyStrings = false) const;
        /// Join substrings with a 'glue' string.
        void Join(const std::vector<String>& subStrings, const String& glue);
        /// Return index to the first occurrence of a string, or NPOS if not found.
        unsigned Find(const String& str, unsigned startPos = 0, bool caseSensitive = true) const;
        /// Return index to the first occurrence of a character, or NPOS if not found.
        unsigned Find(char c, unsigned startPos = 0, bool caseSensitive = true) const;
        /// Return index to the last occurrence of a string, or NPOS if not found.
        unsigned FindLast(const String& str, unsigned startPos = NPOS, bool caseSensitive = true) const;
        /// Return index to the last occurrence of a character, or NPOS if not found.
        unsigned FindLast(char c, unsigned startPos = NPOS, bool caseSensitive = true) const;
        /// Return whether starts with a string.
        bool StartsWith(const String& str, bool caseSensitive = true) const;
        /// Return whether ends with a string.
        bool EndsWith(const String& str, bool caseSensitive = true) const;
        
        /// Return the C string.
        const char* CString() const;
        
        /// Return length.
        unsigned Len() const;
        
        /// Return buffer capacity.
        unsigned Capability() const;
        
        /// Return whether the string is empty.
        bool Empty() const;
        
        /// Return comparison result with a string.
        int Compare(const String& str, bool caseSensitive = true) const;
        /// Return comparison result with a C string.
        int Compare(const char* str, bool caseSensitive = true) const;
        
        /// Return whether contains a specific occurrence of a string.
        bool Contains(const String& str, bool caseSensitive = true) const { return Find(str, 0, caseSensitive) != NPOS; }
        
        /// Return whether contains a specific character.
        bool Contains(char c, bool caseSensitive = true) const { return Find(c, 0, caseSensitive) != NPOS; }
        
        /// Construct UTF8 content from Latin1.
        void SetUTF8FromLatin1(const char* str);
        /// Construct UTF8 content from wide characters.
        void SetUTF8FromWChar(const wchar_t* str);
        /// Calculate number of characters in UTF8 content.
        unsigned LengthUTF8() const;
        /// Return byte offset to char in UTF8 content.
        unsigned ByteOffsetUTF8(unsigned index) const;
        /// Return next Unicode character from UTF8 content and increase byte offset.
        unsigned NextUTF8Char(unsigned& byteOffset) const;
        /// Return Unicode character at index from UTF8 content.
        unsigned AtUTF8(unsigned index) const;
        /// Replace Unicode character at index from UTF8 content.
        void ReplaceUTF8(unsigned index, unsigned unicodeChar);
        /// Append Unicode character at the end as UTF8.
        String& AppendUTF8(unsigned unicodeChar);
        /// Return a UTF8 substring from position to end.
        String SubstringUTF8(unsigned pos) const;
        /// Return a UTF8 substring with length from position.
        String SubstringUTF8(unsigned pos, unsigned length) const;
        
        /// Return hash value for HashSet & HashMap.
        unsigned ToHash() const;
        
        /// Return substrings split by a separator char. By default don't return empty strings.
        static std::vector<String> Split(const char* str, char separator, bool keepEmptyStrings = false);
        /// Return a string by joining substrings with a 'glue' string.
        static String Joined(const std::vector<String>& subStrings, const String& glue);
        /// Encode Unicode character to UTF8. Pointer will be incremented.
        static void EncodeUTF8(char*& dest, unsigned unicodeChar);
        /// Decode Unicode character from UTF8. Pointer will be incremented.
        static unsigned DecodeUTF8(const char*& src);
#ifdef _WIN32
        /// Encode Unicode character to UTF16. Pointer will be incremented.
        static void EncodeUTF16(wchar_t*& dest, unsigned unicodeChar);
        /// Decode Unicode character from UTF16. Pointer will be incremented.
        static unsigned DecodeUTF16(const wchar_t*& src);
#endif
        
        /// Return length of a C string.
        static unsigned CStringLength(const char* str);
        
        /// Append to string using formatting.
        String& AppendWithFormat(const char* formatString, ...);
        /// Append to string using variable arguments.
        String& AppendWithFormatArgs(const char* formatString, va_list args);
        
        /// Compare two C strings.
        static int Compare(const char* str1, const char* str2, bool caseSensitive);
        
    protected:
        /// Move a range of characters within the string.
        void MoveRange(unsigned dest, unsigned src, unsigned count);
        
        /// Replace a substring with another substring.
        void Replace(unsigned pos, unsigned length, const char* srcStart, unsigned srcLength);
    };
    
    /// Add a string to a C string.
    inline String operator +(const char* lhs, const String& rhs)
    {
        String ret(lhs);
        ret += rhs;
        return ret;
    }
    
    /// Add a string to a wide char C string.
    inline String operator +(const wchar_t* lhs, const String& rhs)
    {
        String ret(lhs);
        ret += rhs;
        return ret;
    }
    
    /////////////////////////////////////////////////////////////////////
    
    class WString
    {
    protected:
        /// String allocator.
        IAllocator* Allocator;
        /// String length.
        unsigned Length;
        /// String buffer, null if not allocated.
        wchar_t* Buffer;
    public:
        WString();
        
        WString(const String& str);
        
        ~WString();
        
        wchar_t& operator [](unsigned index);
        
        const wchar_t& operator [](unsigned index) const;
        
        wchar_t& At(unsigned index);
        
        const wchar_t& At(unsigned index) const;
        
        void Resize(unsigned newLength, IAllocator* allocator = nullptr);
        
        bool Empty() const;
        
        unsigned Len() const;
        
        const wchar_t* CString() const;
    };
}

#include "GenericKit/String.inl"
