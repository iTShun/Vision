/////////////////////////////////////////////////////////////////////
#include "GenericKit/String.h"

namespace GenericKit
{
    inline String::String()
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
    }
    
    inline String::String(const String& str)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        *this = str;
    }
    
    inline String::String(const char* str)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        *this = str;
    }
    
    inline String::String(char* str)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        *this = (const char*)str;
    }

    inline String::String(const char* str, unsigned length)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        Resize(length);
        MemCopy(Buffer, str, length);
    }
    
    inline String::String(const wchar_t* str)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        SetUTF8FromWChar(str);
    }
    
    inline String::String(wchar_t* str)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        SetUTF8FromWChar(str);
    }
    
    template <class T>
    inline String::String(const T& value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        *this = value.ToString();
    }
    
    inline String::~String()
    {
        if (Capacity)
        {
            if (Allocator)
                KitFree(Allocator, Buffer);
            else
                delete [] Buffer;
        }
    }
    
    inline String& String::operator =(const String& rhs)
    {
        Resize(rhs.Length);
        MemCopy(Buffer, rhs.Buffer, rhs.Length);
        
        return *this;
    }
    
    inline String& String::operator =(const char* rhs)
    {
        unsigned rhsLength = CStringLength(rhs);
        Resize(rhsLength);
        MemCopy(Buffer, rhs, rhsLength);
        
        return *this;
    }
    
    inline String& String::operator +=(const String& rhs)
    {
        unsigned oldLength = Length;
        Resize(Length + rhs.Length);
        MemCopy(Buffer + oldLength, rhs.Buffer, rhs.Length);
        
        return *this;
    }
    
    inline String& String::operator +=(const char* rhs)
    {
        unsigned rhsLength = CStringLength(rhs);
        unsigned oldLength = Length;
        Resize(Length + rhsLength);
        MemCopy(Buffer + oldLength, rhs, rhsLength);
        
        return *this;
    }
    
    inline String& String::operator +=(char rhs)
    {
        unsigned oldLength = Length;
        Resize(Length + 1);
        Buffer[oldLength] = rhs;
        
        return *this;
    }
    
    template <class T>
    inline String String::operator +=(const T& rhs)
    {
        return *this += rhs.ToString();
    }
    
    inline String String::operator +(const String& rhs) const
    {
        String ret;
        ret.Resize(Length + rhs.Length);
        MemCopy(ret.Buffer, Buffer, Length);
        MemCopy(ret.Buffer + Length, rhs.Buffer, rhs.Length);
        
        return ret;
    }
    
    inline String String::operator +(const char* rhs) const
    {
        unsigned rhsLength = CStringLength(rhs);
        String ret;
        ret.Resize(Length + rhsLength);
        MemCopy(ret.Buffer, Buffer, Length);
        MemCopy(ret.Buffer + Length, rhs, rhsLength);
        
        return ret;
    }
    
    inline bool String::operator ==(const String& rhs) const
    {
        return strcmp(CString(), rhs.CString()) == 0;
    }
    
    inline bool String::operator !=(const String& rhs) const
    {
        return strcmp(CString(), rhs.CString()) != 0;
    }
    
    inline bool String::operator <(const String& rhs) const
    {
        return strcmp(CString(), rhs.CString()) < 0;
    }
    
    inline bool String::operator >(const String& rhs) const
    {
        return strcmp(CString(), rhs.CString()) > 0;
    }
    
    inline bool String::operator ==(const char* rhs) const
    {
        return strcmp(CString(), rhs) == 0;
    }
    
    inline bool String::operator !=(const char* rhs) const
    {
        return strcmp(CString(), rhs) != 0;
    }
    
    inline bool String::operator <(const char* rhs) const
    {
        return strcmp(CString(), rhs) < 0;
    }
    
    inline bool String::operator >(const char* rhs) const
    {
        return strcmp(CString(), rhs) > 0;
    }
    
    inline char& String::operator [](unsigned index)
    {
        assert(index < Length);
        return Buffer[index];
    }
    
    inline const char& String::operator [](unsigned index) const
    {
        assert(index < Length);
        return Buffer[index];
    }
    
    inline char& String::At(unsigned index)
    {
        assert(index < Length);
        return Buffer[index];
    }
    
    inline const char& String::At(unsigned index) const
    {
        assert(index < Length);
        return Buffer[index];
    }
    
    inline char String::Front() const
    {
        return Buffer[0];
        
    }
    
    inline char String::Back() const
    {
        return Length ? Buffer[Length - 1] : Buffer[0];
    }
    
    inline const char* String::CString() const
    {
        return Buffer;
    }
    
    inline unsigned String::Len() const
    {
        return Length;
    }
    
    inline unsigned String::Capability() const
    {
        return Capacity;
    }
    
    inline bool String::Empty() const
    {
        return Length == 0;
    }
    
    inline unsigned String::ToHash() const
    {
        unsigned hash = 0;
        const char* ptr = Buffer;
        while (*ptr)
        {
            hash = *ptr + (hash << 6) + (hash << 16) - hash;
            ++ptr;
        }
        
        return hash;
    }
    
    /// Return length of a C string.
    inline unsigned String::CStringLength(const char* str) { return str ? (unsigned)StrLen(str) : 0; }

    inline void String::MoveRange(unsigned dest, unsigned src, unsigned count)
    {
        if (count)
            MemMove(Buffer + dest, Buffer + src, count);
    }
    
    /////////////////////////////////////////////////////////////////////
    
    inline wchar_t& WString::operator [](unsigned index)
    {
        assert(index < Length);
        return Buffer[index];
    }
    
    inline const wchar_t& WString::operator [](unsigned index) const
    {
        assert(index < Length);
        return Buffer[index];
    }
    
    inline wchar_t& WString::At(unsigned index)
    {
        assert(index < Length);
        return Buffer[index];
    }
    
    inline const wchar_t& WString::At(unsigned index) const
    {
        assert(index < Length);
        return Buffer[index];
    }
    
    inline bool WString::Empty() const
    {
        return Length == 0;
    }
    
    inline unsigned WString::Len() const
    {
        return Length;
    }
    
    inline const wchar_t* WString::CString() const
    {
        return Buffer;
    }
}
