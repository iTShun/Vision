#include "String.h"
#include "GenericKit/Debug.h"

namespace GenericKit
{
    char String::EndZero = 0;

    const String String::EMPTY;

    String::String(const WString& str)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        SetUTF8FromWChar(str.CString());
    }

    String::String(int value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%d", value);
        *this = tempBuffer;
    }

    String::String(short value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%d", value);
        *this = tempBuffer;
    }

    String::String(long value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%ld", value);
        *this = tempBuffer;
    }

    String::String(long long value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%lld", value);
        *this = tempBuffer;
    }

    String::String(unsigned value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%u", value);
        *this = tempBuffer;
    }

    String::String(unsigned short value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%u", value);
        *this = tempBuffer;
    }

    String::String(unsigned long value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%lu", value);
        *this = tempBuffer;
    }

    String::String(unsigned long long value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%llu", value);
        *this = tempBuffer;
    }

    String::String(float value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%g", value);
        *this = tempBuffer;
    }

    String::String(double value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        Sprintf(tempBuffer, "%.15g", value);
        *this = tempBuffer;
    }

    String::String(bool value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        if (value)
            *this = "true";
        else
            *this = "false";
    }

    String::String(char value)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        Resize(1);
        Buffer[0] = value;
    }

    String::String(char value, unsigned length)
    : Length(0)
    , Capacity(0)
    , Buffer(&EndZero)
    {
        Resize(length);
        for (unsigned i = 0; i < length; ++i)
            Buffer[i] = value;
    }
    
    String& String::operator +=(int rhs)
    {
        return *this += String(rhs);
    }
    
    String& String::operator +=(short rhs)
    {
        return *this += String(rhs);
    }
    
    String& String::operator +=(long rhs)
    {
        return *this += String(rhs);
    }
    
    String& String::operator +=(long long rhs)
    {
        return *this += String(rhs);
    }
    
    String& String::operator +=(unsigned rhs)
    {
        return *this += String(rhs);
    }
    
    String& String::operator +=(unsigned short rhs)
    {
        return *this += String(rhs);
    }
    
    String& String::operator +=(unsigned long rhs)
    {
        return *this += String(rhs);
    }
    
    String& String::operator +=(unsigned long long rhs)
    {
        return *this += String(rhs);
    }
    
    String& String::operator +=(float rhs)
    {
        return *this += String(rhs);
    }
    
    String& String::operator +=(bool rhs)
    {
        return *this += String(rhs);
    }

    void String::Replace(char replaceThis, char replaceWith, bool caseSensitive)
    {
        if (caseSensitive)
        {
            for (unsigned i = 0; i < Length; ++i)
            {
                if (Buffer[i] == replaceThis)
                    Buffer[i] = replaceWith;
            }
        }
        else
        {
            replaceThis = (char)tolower(replaceThis);
            for (unsigned i = 0; i < Length; ++i)
            {
                if (tolower(Buffer[i]) == replaceThis)
                    Buffer[i] = replaceWith;
            }
        }
    }
    
    void String::Replace(const String& replaceThis, const String& replaceWith, bool caseSensitive)
    {
        unsigned nextPos = 0;
        
        while (nextPos < Length)
        {
            unsigned pos = Find(replaceThis, nextPos, caseSensitive);
            if (pos == NPOS)
                break;
            Replace(pos, replaceThis.Length, replaceWith);
            nextPos = pos + replaceWith.Length;
        }
    }
    
    void String::Replace(unsigned pos, unsigned length, const String& replaceWith)
    {
        // If substring is illegal, do nothing
        if (pos + length > Length)
            return;
        
        Replace(pos, length, replaceWith.Buffer, replaceWith.Length);
    }
    
    void String::Replace(unsigned pos, unsigned length, const char* replaceWith)
    {
        // If substring is illegal, do nothing
        if (pos + length > Length)
            return;
        
        Replace(pos, length, replaceWith, CStringLength(replaceWith));
    }
    
    String String::Replaced(char replaceThis, char replaceWith, bool caseSensitive) const
    {
        String ret(*this);
        ret.Replace(replaceThis, replaceWith, caseSensitive);
        return ret;
    }
    
    String String::Replaced(const String& replaceThis, const String& replaceWith, bool caseSensitive) const
    {
        String ret(*this);
        ret.Replace(replaceThis, replaceWith, caseSensitive);
        return ret;
    }
    
    String& String::Append(const String& str)
    {
        return *this += str;
    }
    
    String& String::Append(const char* str)
    {
        return *this += str;
    }
    
    String& String::Append(char c)
    {
        return *this += c;
    }
    
    String& String::Append(const char* str, unsigned length)
    {
        if (str)
        {
            unsigned oldLength = Length;
            Resize(oldLength + length);
            MemCopy(&Buffer[oldLength], str, length);
        }
        return *this;
    }
    
    void String::Insert(unsigned pos, const String& str)
    {
        if (pos > Length)
            pos = Length;
        
        if (pos == Length)
            (*this) += str;
        else
            Replace(pos, 0, str);
    }
    
    void String::Insert(unsigned pos, char c)
    {
        if (pos > Length)
            pos = Length;
        
        if (pos == Length)
            (*this) += c;
        else
        {
            unsigned oldLength = Length;
            Resize(Length + 1);
            MoveRange(pos + 1, pos, oldLength - pos);
            Buffer[pos] = c;
        }
    }
    
    void String::Erase(unsigned pos, unsigned length)
    {
        Replace(pos, length, String::EMPTY);
    }
    
    void String::Resize(unsigned newLength, IAllocator* allocator)
    {
        if (!Allocator)
            Allocator = allocator;
        
        if (!Capacity)
        {
            // If zero length requested, do not allocate buffer yet
            if (!newLength)
                return;
            
            // Calculate initial capacity
            Capacity = newLength + 1;
            if (Capacity < MIN_CAPACITY)
                Capacity = MIN_CAPACITY;
            
            if (Allocator)
                Buffer = (char*)KitAlloc(Allocator, Capacity);
            else
                Buffer = new char[Capacity];
        }
        else
        {
            if (newLength && Capacity < newLength + 1)
            {
                // Increase the capacity with half each time it is exceeded
                while (Capacity < newLength + 1)
                    Capacity += (Capacity + 1) >> 1;
                
                char* newBuffer = 0;
                
                if (Allocator)
                    newBuffer = (char*)KitAlloc(Allocator, Capacity);
                else
                    newBuffer = new char[Capacity];
                
                // Move the existing data to the new buffer, then delete the old buffer
                if (Length)
                    MemCopy(newBuffer, Buffer, Length);
                
                if (Allocator)
                    KitFree(Allocator, Buffer);
                else
                    delete[] Buffer;
                
                Buffer = newBuffer;
            }
        }
        
        Buffer[newLength] = 0;
        Length = newLength;
    }
    
    void String::Reserve(unsigned newCapacity)
    {
        if (newCapacity < Length + 1)
            newCapacity = Length + 1;
        if (newCapacity == Capacity)
            return;
        
        char* newBuffer = 0;
        
        if (Allocator)
            newBuffer = (char*)KitAlloc(Allocator, newCapacity);
        else
            newBuffer = new char[newCapacity];
        // Move the existing data to the new buffer, then delete the old buffer
        MemCopy(newBuffer, Buffer, Length + 1);
        if (Capacity)
            if (Allocator)
                KitFree(Allocator, Buffer);
            else
                delete[] Buffer;
        
        Capacity = newCapacity;
        Buffer = newBuffer;
    }
    
    void String::Compact()
    {
        if (Capacity)
            Reserve(Length + 1);
    }
    
    void String::Clear()
    {
        Resize(0);
    }
    
    void String::Swap(String& str)
    {
        (std::swap)(Length, str.Length);
        (std::swap)(Capacity, str.Capacity);
        (std::swap)(Buffer, str.Buffer);
    }
    
    String String::Substring(unsigned pos) const
    {
        if (pos < Length)
        {
            String ret;
            ret.Resize(Length - pos);
            MemCopy(ret.Buffer, Buffer + pos, ret.Length);
            
            return ret;
        }
        else
            return String();
    }
    
    String String::Substring(unsigned pos, unsigned length) const
    {
        if (pos < Length)
        {
            String ret;
            if (pos + length > Length)
                length = Length - pos;
            ret.Resize(length);
            MemCopy(ret.Buffer, Buffer + pos, ret.Length);
            
            return ret;
        }
        else
            return String();
    }
    
    String String::Trimmed() const
    {
        unsigned trimStart = 0;
        unsigned trimEnd = Length;
        
        while (trimStart < trimEnd)
        {
            char c = Buffer[trimStart];
            if (c != ' ' && c != 9)
                break;
            ++trimStart;
        }
        while (trimEnd > trimStart)
        {
            char c = Buffer[trimEnd - 1];
            if (c != ' ' && c != 9)
                break;
            --trimEnd;
        }
        
        return Substring(trimStart, trimEnd - trimStart);
    }
    
    String String::ToLower() const
    {
        String ret(*this);
        for (unsigned i = 0; i < ret.Length; ++i)
            ret[i] = (char)tolower(Buffer[i]);
        
        return ret;
    }
    
    String String::ToUpper() const
    {
        String ret(*this);
        for (unsigned i = 0; i < ret.Length; ++i)
            ret[i] = (char)toupper(Buffer[i]);
        
        return ret;
    }
    
    std::vector<String> String::Split(char separator, bool keepEmptyStrings) const
    {
        return Split(CString(), separator, keepEmptyStrings);
    }
    
    void String::Join(const std::vector<String>& subStrings, const String& glue)
    {
        *this = Joined(subStrings, glue);
    }
    
    unsigned String::Find(char c, unsigned startPos, bool caseSensitive) const
    {
        if (caseSensitive)
        {
            for (unsigned i = startPos; i < Length; ++i)
            {
                if (Buffer[i] == c)
                    return i;
            }
        }
        else
        {
            c = (char)tolower(c);
            for (unsigned i = startPos; i < Length; ++i)
            {
                if (tolower(Buffer[i]) == c)
                    return i;
            }
        }
        
        return NPOS;
    }
    
    unsigned String::Find(const String& str, unsigned startPos, bool caseSensitive) const
    {
        if (!str.Length || str.Length > Length)
            return NPOS;
        
        char first = str.Buffer[0];
        if (!caseSensitive)
            first = (char)tolower(first);
        
        for (unsigned i = startPos; i <= Length - str.Length; ++i)
        {
            char c = Buffer[i];
            if (!caseSensitive)
                c = (char)tolower(c);
            
            if (c == first)
            {
                unsigned skip = NPOS;
                bool found = true;
                for (unsigned j = 1; j < str.Length; ++j)
                {
                    c = Buffer[i + j];
                    char d = str.Buffer[j];
                    if (!caseSensitive)
                    {
                        c = (char)tolower(c);
                        d = (char)tolower(d);
                    }
                    
                    if (skip == NPOS && c == first)
                        skip = i + j - 1;
                    
                    if (c != d)
                    {
                        found = false;
                        if (skip != NPOS)
                            i = skip;
                        break;
                    }
                }
                if (found)
                    return i;
            }
        }
        
        return NPOS;
    }
    
    unsigned String::FindLast(char c, unsigned startPos, bool caseSensitive) const
    {
        if (startPos >= Length)
            startPos = Length - 1;
        
        if (caseSensitive)
        {
            for (unsigned i = startPos; i < Length; --i)
            {
                if (Buffer[i] == c)
                    return i;
            }
        }
        else
        {
            c = (char)tolower(c);
            for (unsigned i = startPos; i < Length; --i)
            {
                if (tolower(Buffer[i]) == c)
                    return i;
            }
        }
        
        return NPOS;
    }
    
    unsigned String::FindLast(const String& str, unsigned startPos, bool caseSensitive) const
    {
        if (!str.Length || str.Length > Length)
            return NPOS;
        if (startPos > Length - str.Length)
            startPos = Length - str.Length;
        
        char first = str.Buffer[0];
        if (!caseSensitive)
            first = (char)tolower(first);
        
        for (unsigned i = startPos; i < Length; --i)
        {
            char c = Buffer[i];
            if (!caseSensitive)
                c = (char)tolower(c);
            
            if (c == first)
            {
                bool found = true;
                for (unsigned j = 1; j < str.Length; ++j)
                {
                    c = Buffer[i + j];
                    char d = str.Buffer[j];
                    if (!caseSensitive)
                    {
                        c = (char)tolower(c);
                        d = (char)tolower(d);
                    }
                    
                    if (c != d)
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                    return i;
            }
        }
        
        return NPOS;
    }
    
    bool String::StartsWith(const String& str, bool caseSensitive) const
    {
        return Find(str, 0, caseSensitive) == 0;
    }
    
    bool String::EndsWith(const String& str, bool caseSensitive) const
    {
        unsigned pos = FindLast(str, Len() - 1, caseSensitive);
        return pos != NPOS && pos == Len() - str.Len();
    }
    
    int String::Compare(const String& str, bool caseSensitive) const
    {
        return Compare(CString(), str.CString(), caseSensitive);
    }
    
    int String::Compare(const char* str, bool caseSensitive) const
    {
        return Compare(CString(), str, caseSensitive);
    }
    
    void String::SetUTF8FromLatin1(const char* str)
    {
        char temp[7];
        
        Clear();
        
        if (!str)
            return;
        
        while (*str)
        {
            char* dest = temp;
            EncodeUTF8(dest, (unsigned)*str++);
            *dest = 0;
            Append(temp);
        }
    }
    
    void String::SetUTF8FromWChar(const wchar_t* str)
    {
        char temp[7];
        
        Clear();
        
        if (!str)
            return;
        
#ifdef _WIN32
        while (*str)
        {
            unsigned unicodeChar = DecodeUTF16(str);
            char* dest = temp;
            EncodeUTF8(dest, unicodeChar);
            *dest = 0;
            Append(temp);
        }
#else
        while (*str)
        {
            char* dest = temp;
            EncodeUTF8(dest, (unsigned)*str++);
            *dest = 0;
            Append(temp);
        }
#endif
    }
    
    unsigned String::LengthUTF8() const
    {
        unsigned ret = 0;
        
        const char* src = Buffer;
        if (!src)
            return ret;
        const char* end = Buffer + Length;
        
        while (src < end)
        {
            DecodeUTF8(src);
            ++ret;
        }
        
        return ret;
    }
    
    unsigned String::ByteOffsetUTF8(unsigned index) const
    {
        unsigned byteOffset = 0;
        unsigned utfPos = 0;
        
        while (utfPos < index && byteOffset < Length)
        {
            NextUTF8Char(byteOffset);
            ++utfPos;
        }
        
        return byteOffset;
    }
    
    unsigned String::NextUTF8Char(unsigned& byteOffset) const
    {
        if (!Buffer)
            return 0;
        
        const char* src = Buffer + byteOffset;
        unsigned ret = DecodeUTF8(src);
        byteOffset = (unsigned)(src - Buffer);
        
        return ret;
    }
    
    unsigned String::AtUTF8(unsigned index) const
    {
        unsigned byteOffset = ByteOffsetUTF8(index);
        return NextUTF8Char(byteOffset);
    }
    
    void String::ReplaceUTF8(unsigned index, unsigned unicodeChar)
    {
        unsigned utfPos = 0;
        unsigned byteOffset = 0;
        
        while (utfPos < index && byteOffset < Length)
        {
            NextUTF8Char(byteOffset);
            ++utfPos;
        }
        
        if (utfPos < index)
            return;
        
        unsigned beginCharPos = byteOffset;
        NextUTF8Char(byteOffset);
        
        char temp[7];
        char* dest = temp;
        EncodeUTF8(dest, unicodeChar);
        *dest = 0;
        
        Replace(beginCharPos, byteOffset - beginCharPos, temp, (unsigned)(dest - temp));
    }
    
    String& String::AppendUTF8(unsigned unicodeChar)
    {
        char temp[7];
        char* dest = temp;
        EncodeUTF8(dest, unicodeChar);
        *dest = 0;
        return Append(temp);
    }
    
    String String::SubstringUTF8(unsigned pos) const
    {
        unsigned utf8Length = LengthUTF8();
        unsigned byteOffset = ByteOffsetUTF8(pos);
        String ret;
        
        while (pos < utf8Length)
        {
            ret.AppendUTF8(NextUTF8Char(byteOffset));
            ++pos;
        }
        
        return ret;
    }
    
    String String::SubstringUTF8(unsigned pos, unsigned length) const
    {
        unsigned utf8Length = LengthUTF8();
        unsigned byteOffset = ByteOffsetUTF8(pos);
        unsigned endPos = pos + length;
        String ret;
        
        while (pos < endPos && pos < utf8Length)
        {
            ret.AppendUTF8(NextUTF8Char(byteOffset));
            ++pos;
        }
        
        return ret;
    }
    
    void String::EncodeUTF8(char*& dest, unsigned unicodeChar)
    {
        if (unicodeChar < 0x80)
            *dest++ = unicodeChar;
        else if (unicodeChar < 0x800)
        {
            dest[0] = (char)(0xc0 | ((unicodeChar >> 6) & 0x1f));
            dest[1] = (char)(0x80 | (unicodeChar & 0x3f));
            dest += 2;
        }
        else if (unicodeChar < 0x10000)
        {
            dest[0] = (char)(0xe0 | ((unicodeChar >> 12) & 0xf));
            dest[1] = (char)(0x80 | ((unicodeChar >> 6) & 0x3f));
            dest[2] = (char)(0x80 | (unicodeChar & 0x3f));
            dest += 3;
        }
        else if (unicodeChar < 0x200000)
        {
            dest[0] = (char)(0xf0 | ((unicodeChar >> 18) & 0x7));
            dest[1] = (char)(0x80 | ((unicodeChar >> 12) & 0x3f));
            dest[2] = (char)(0x80 | ((unicodeChar >> 6) & 0x3f));
            dest[3] = (char)(0x80 | (unicodeChar & 0x3f));
            dest += 4;
        }
        else if (unicodeChar < 0x4000000)
        {
            dest[0] = (char)(0xf8 | ((unicodeChar >> 24) & 0x3));
            dest[1] = (char)(0x80 | ((unicodeChar >> 18) & 0x3f));
            dest[2] = (char)(0x80 | ((unicodeChar >> 12) & 0x3f));
            dest[3] = (char)(0x80 | ((unicodeChar >> 6) & 0x3f));
            dest[4] = (char)(0x80 | (unicodeChar & 0x3f));
            dest += 5;
        }
        else
        {
            dest[0] = (char)(0xfc | ((unicodeChar >> 30) & 0x1));
            dest[1] = (char)(0x80 | ((unicodeChar >> 24) & 0x3f));
            dest[2] = (char)(0x80 | ((unicodeChar >> 18) & 0x3f));
            dest[3] = (char)(0x80 | ((unicodeChar >> 12) & 0x3f));
            dest[4] = (char)(0x80 | ((unicodeChar >> 6) & 0x3f));
            dest[5] = (char)(0x80 | (unicodeChar & 0x3f));
            dest += 6;
        }
    }
    
#define GET_NEXT_CONTINUATION_BYTE(ptr) *ptr; if ((unsigned char)*ptr < 0x80 || (unsigned char)*ptr >= 0xc0) return '?'; else ++ptr;
    
    unsigned String::DecodeUTF8(const char*& src)
    {
        if (src == 0)
            return 0;
        
        unsigned char char1 = *src++;
        
        // Check if we are in the middle of a UTF8 character
        if (char1 >= 0x80 && char1 < 0xc0)
        {
            while ((unsigned char)*src >= 0x80 && (unsigned char)*src < 0xc0)
                ++src;
            return '?';
        }
        
        if (char1 < 0x80)
            return char1;
        else if (char1 < 0xe0)
        {
            unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
            return (unsigned)((char2 & 0x3f) | ((char1 & 0x1f) << 6));
        }
        else if (char1 < 0xf0)
        {
            unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char3 = GET_NEXT_CONTINUATION_BYTE(src);
            return (unsigned)((char3 & 0x3f) | ((char2 & 0x3f) << 6) | ((char1 & 0xf) << 12));
        }
        else if (char1 < 0xf8)
        {
            unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char3 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char4 = GET_NEXT_CONTINUATION_BYTE(src);
            return (unsigned)((char4 & 0x3f) | ((char3 & 0x3f) << 6) | ((char2 & 0x3f) << 12) | ((char1 & 0x7) << 18));
        }
        else if (char1 < 0xfc)
        {
            unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char3 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char4 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char5 = GET_NEXT_CONTINUATION_BYTE(src);
            return (unsigned)((char5 & 0x3f) | ((char4 & 0x3f) << 6) | ((char3 & 0x3f) << 12) | ((char2 & 0x3f) << 18) |
                              ((char1 & 0x3) << 24));
        }
        else
        {
            unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char3 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char4 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char5 = GET_NEXT_CONTINUATION_BYTE(src);
            unsigned char char6 = GET_NEXT_CONTINUATION_BYTE(src);
            return (unsigned)((char6 & 0x3f) | ((char5 & 0x3f) << 6) | ((char4 & 0x3f) << 12) | ((char3 & 0x3f) << 18) |
                              ((char2 & 0x3f) << 24) | ((char1 & 0x1) << 30));
        }
    }
    
#ifdef _WIN32
    void String::EncodeUTF16(wchar_t*& dest, unsigned unicodeChar)
    {
        if (unicodeChar < 0x10000)
            *dest++ = unicodeChar;
        else
        {
            unicodeChar -= 0x10000;
            *dest++ = 0xd800 | ((unicodeChar >> 10) & 0x3ff);
            *dest++ = 0xdc00 | (unicodeChar & 0x3ff);
        }
    }
    
    unsigned String::DecodeUTF16(const wchar_t*& src)
    {
        if (src == 0)
            return 0;
        
        unsigned short word1 = *src++;
        
        // Check if we are at a low surrogate
        if (word1 >= 0xdc00 && word1 < 0xe000)
        {
            while (*src >= 0xdc00 && *src < 0xe000)
                ++src;
            return '?';
        }
        
        if (word1 < 0xd800 || word1 >= 0xe000)
            return word1;
        else
        {
            unsigned short word2 = *src++;
            if (word2 < 0xdc00 || word2 >= 0xe000)
            {
                --src;
                return '?';
            }
            else
                return (((word1 & 0x3ff) << 10) | (word2 & 0x3ff)) + 0x10000;
        }
    }
#endif
    
    std::vector<String> String::Split(const char* str, char separator, bool keepEmptyStrings)
    {
        std::vector<String> ret;
        const char* strEnd = str + String::CStringLength(str);
        
        for (const char* splitEnd = str; splitEnd != strEnd; ++splitEnd)
        {
            if (*splitEnd == separator)
            {
                const ptrdiff_t splitLen = splitEnd - str;
                if (splitLen > 0 || keepEmptyStrings)
                    ret.push_back(String(str, splitLen));
                str = splitEnd + 1;
            }
        }
        
        const ptrdiff_t splitLen = strEnd - str;
        if (splitLen > 0 || keepEmptyStrings)
            ret.push_back(String(str, splitLen));
        
        return ret;
    }
    
    String String::Joined(const std::vector<String>& subStrings, const String& glue)
    {
        if (subStrings.empty())
            return String();
        
        String joinedString(subStrings[0]);
        for (unsigned i = 1; i < subStrings.size(); ++i)
            joinedString.Append(glue).Append(subStrings[i]);
        
        return joinedString;
    }
    
    String& String::AppendWithFormat(const char* formatString, ...)
    {
        va_list args;
        va_start(args, formatString);
        AppendWithFormatArgs(formatString, args);
        va_end(args);
        return *this;
    }
    
    String& String::AppendWithFormatArgs(const char* formatString, va_list args)
    {
        int pos = 0, lastPos = 0;
        int length = (int)StrLen(formatString);
        
        while (true)
        {
            // Scan the format string and find %a argument where a is one of d, f, s ...
            while (pos < length && formatString[pos] != '%') pos++;
            Append(formatString + lastPos, (unsigned)(pos - lastPos));
            if (pos >= length)
                return *this;
            
            char format = formatString[pos + 1];
            pos += 2;
            lastPos = pos;
            
            switch (format)
            {
                    // Integer
                case 'd':
                case 'i':
                {
                    int arg = va_arg(args, int);
                    Append(String(arg));
                    break;
                }
                    
                    // Unsigned
                case 'u':
                {
                    unsigned arg = va_arg(args, unsigned);
                    Append(String(arg));
                    break;
                }
                    
                    // Unsigned long
                case 'l':
                {
                    unsigned long arg = va_arg(args, unsigned long);
                    Append(String(arg));
                    break;
                }
                    
                    // Real
                case 'f':
                {
                    double arg = va_arg(args, double);
                    Append(String(arg));
                    break;
                }
                    
                    // Character
                case 'c':
                {
                    int arg = va_arg(args, int);
                    Append((char)arg);
                    break;
                }
                    
                    // C string
                case 's':
                {
                    char* arg = va_arg(args, char*);
                    Append(arg);
                    break;
                }
                    
                    // Hex
                case 'x':
                {
                    char buf[CONVERSION_BUFFER_LENGTH];
                    int arg = va_arg(args, int);
                    int arglen = ::sprintf(buf, "%x", arg);
                    Append(buf, (unsigned)arglen);
                    break;
                }
                    
                    // Pointer
                case 'p':
                {
                    char buf[CONVERSION_BUFFER_LENGTH];
                    int arg = va_arg(args, int);
                    int arglen = ::sprintf(buf, "%p", reinterpret_cast<void*>(arg));
                    Append(buf, (unsigned)arglen);
                    break;
                }
                    
                case '%':
                {
                    Append("%", 1);
                    break;
                }
                    
                default:
                    DebugPrintf("Unsupported format specifier: '%c'", format);
                    break;
            }
        }
    }
    
    int String::Compare(const char* lhs, const char* rhs, bool caseSensitive)
    {
        if (!lhs || !rhs)
            return lhs ? 1 : (rhs ? -1 : 0);
        
        if (caseSensitive)
            return strcmp(lhs, rhs);
        else
        {
            for (;;)
            {
                char l = (char)tolower(*lhs);
                char r = (char)tolower(*rhs);
                if (!l || !r)
                    return l ? 1 : (r ? -1 : 0);
                if (l < r)
                    return -1;
                if (l > r)
                    return 1;
                
                ++lhs;
                ++rhs;
            }
        }
    }
    
    void String::Replace(unsigned pos, unsigned length, const char* srcStart, unsigned srcLength)
    {
        int delta = (int)srcLength - (int)length;
        
        if (pos + length < Length)
        {
            if (delta < 0)
            {
                MoveRange(pos + srcLength, pos + length, Length - pos - length);
                Resize(Length + delta);
            }
            if (delta > 0)
            {
                Resize(Length + delta);
                MoveRange(pos + srcLength, pos + length, Length - pos - length - delta);
            }
        }
        else
            Resize(Length + delta);
        
        MemCopy(Buffer + pos, srcStart, srcLength);
    }
    
    WString::WString() :
    Length(0),
    Buffer(0)
    {
    }
    
    WString::WString(const String& str) :
    Length(0),
    Buffer(0)
    {
#ifdef _WIN32
        unsigned neededSize = 0;
        wchar_t temp[3];
        
        unsigned byteOffset = 0;
        while (byteOffset < str.Length())
        {
            wchar_t* dest = temp;
            String::EncodeUTF16(dest, str.NextUTF8Char(byteOffset));
            neededSize += dest - temp;
        }
        
        Resize(neededSize);
        
        byteOffset = 0;
        wchar_t* dest = Buffer;
        while (byteOffset < str.Length())
            String::EncodeUTF16(dest, str.NextUTF8Char(byteOffset));
#else
        Resize(str.LengthUTF8());
        
        unsigned byteOffset = 0;
        wchar_t* dest = Buffer;
        while (byteOffset < str.Len())
            *dest++ = (wchar_t)str.NextUTF8Char(byteOffset);
#endif
    }
    
    WString::~WString()
    {
        if (Allocator)
            KitFree(Allocator, Buffer);
        else
            delete[] Buffer;
    }
    
    void WString::Resize(unsigned newLength, IAllocator* allocator)
    {
        if (!Allocator)
            Allocator = allocator;
        
        if (!newLength)
        {
            if (Allocator)
                KitFree(Allocator, Buffer);
            else
                delete[] Buffer;
            Buffer = 0;
            Length = 0;
        }
        else
        {
            wchar_t* newBuffer = 0;
            
            if (Allocator)
                newBuffer = (wchar_t*)KitAlloc(Allocator, sizeof(wchar_t) * (newLength + 1));
            else
                newBuffer = new wchar_t[newLength + 1];
            
            if (Buffer)
            {
                unsigned copyLength = Length < newLength ? Length : newLength;
                MemCopy(newBuffer, Buffer, copyLength * sizeof(wchar_t));
                if (Allocator)
                    KitFree(Allocator, Buffer);
                else
                    delete[] Buffer;
            }
            newBuffer[newLength] = 0;
            Buffer = newBuffer;
            Length = newLength;
        }
    }
}
