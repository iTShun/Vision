#include "StringUtils.h"

#if !CRT_NONE
#    include <stdio.h> // vsnprintf
#endif // !CRT_NONE

namespace GenericKit
{
    typedef char (*CharFn)(char _ch);
    
    inline char ToNoop(char _ch)
    {
        return _ch;
    }
    
    inline bool IsInRange(char _ch, char _from, char _to)
    {
        return unsigned(_ch - _from) <= unsigned(_to - _from);
    }
    
    bool IsSpace(char _ch)
    {
        return ' '  == _ch
        || '\t' == _ch
        || '\n' == _ch
        || '\v' == _ch
        || '\f' == _ch
        || '\r' == _ch
        ;
    }
    
    bool IsUpper(char _ch)
    {
        return IsInRange(_ch, 'A', 'Z');
    }
    
    bool IsLower(char _ch)
    {
        return IsInRange(_ch, 'a', 'z');
    }
    
    bool IsAlpha(char _ch)
    {
        return IsLower(_ch) || IsUpper(_ch);
    }
    
    bool IsNumeric(char _ch)
    {
        return IsInRange(_ch, '0', '9');
    }
    
    bool IsAlphaNum(char _ch)
    {
        return false
        || IsAlpha(_ch)
        || IsNumeric(_ch)
        ;
    }
    
    bool IsHexNum(char _ch)
    {
        return false
        || IsInRange(ToLower(_ch), 'a', 'f')
        || IsNumeric(_ch)
        ;
    }
    
    bool IsPrint(char _ch)
    {
        return IsInRange(_ch, ' ', '~');
    }
    
    char ToLower(char _ch)
    {
        return _ch + (IsUpper(_ch) ? 0x20 : 0);
    }
    
    void ToLowerUnsafe(char* _inOutStr, int32_t _len)
    {
        for (int32_t ii = 0; ii < _len; ++ii)
        {
            *_inOutStr = ToLower(*_inOutStr);
        }
    }
    
    void ToLower(char* _inOutStr, int32_t _max)
    {
        const int32_t len = StrLen(_inOutStr, _max);
        ToLowerUnsafe(_inOutStr, len);
    }
    
    char ToUpper(char _ch)
    {
        return _ch - (IsLower(_ch) ? 0x20 : 0);
    }
    
    void ToUpperUnsafe(char* _inOutStr, int32_t _len)
    {
        for (int32_t ii = 0; ii < _len; ++ii)
        {
            *_inOutStr = ToUpper(*_inOutStr);
        }
    }
    
    void ToUpper(char* _inOutStr, int32_t _max)
    {
        const int32_t len = StrLen(_inOutStr, _max);
        ToUpperUnsafe(_inOutStr, len);
    }
    
    int32_t StrLen(const char* _str, int32_t _max)
    {
        if (NULL == _str)
        {
            return 0;
        }
        
        const char* ptr = _str;
        for (; 0 < _max && *ptr != '\0'; ++ptr, --_max) {};
        return int32_t(ptr - _str);
    }
    
    const char* Strws(const char* _str)
    {
        for (; IsSpace(*_str); ++_str) {};
        return _str;
    }
    
    const char* Strnws(const char* _str)
    {
        for (; !IsSpace(*_str); ++_str) {};
        return _str;
    }
    
    const char* StrSkipWord(const char* _str, int32_t _max)
    {
        for (char ch = *_str++; 0 < _max && (IsAlphaNum(ch) || '_' == ch); ch = *_str++, --_max) {};
        return _str-1;
    }
    
    const char* Strmb(const char* _str, char _open, char _close)
    {
        int count = 0;
        for (char ch = *_str++; ch != '\0' && count >= 0; ch = *_str++)
        {
            if (ch == _open)
            {
                count++;
            }
            else if (ch == _close)
            {
                count--;
                if (0 == count)
                {
                    return _str-1;
                }
            }
        }
        
        return NULL;
    }
    
    void EolLF(char* _out, int32_t _size, const char* _str)
    {
        if (0 < _size)
        {
            char* end = _out + _size - 1;
            for (char ch = *_str++; ch != '\0' && _out < end; ch = *_str++)
            {
                if ('\r' != ch)
                {
                    *_out++ = ch;
                }
            }
            
            *_out = '\0';
        }
    }
    
    int32_t Vsnprintf(char* _out, int32_t _max, const char* _format, va_list _argList)
    {
        va_list argList;
        va_copy(argList, _argList);
        int32_t total = 0;
#if CRT_NONE
        total = vsnprintfRef(_out, _max, _format, argList);
#elif CRT_MSVC
        int32_t len = -1;
        if (NULL != _out)
        {
            va_list argListCopy;
            va_copy(argListCopy, _argList);
            len = ::vsnprintf_s(_out, _max, size_t(-1), _format, argListCopy);
            va_end(argListCopy);
        }
        total = -1 == len ? ::_vscprintf(_format, argList) : len;
#else
        total = ::vsnprintf(_out, _max, _format, argList);
#endif // COMPILER_MSVC
        va_end(argList);
        return total;
    }
    
    int32_t Snprintf(char* _out, int32_t _max, const char* _format, ...)
    {
        va_list argList;
        va_start(argList, _format);
        int32_t total = Vsnprintf(_out, _max, _format, argList);
        va_end(argList);
        return total;
    }
    
    int32_t Sprintf(char* _out, const char* _format, ...)
    {
        va_list argList;
        va_start(argList, _format);
        int32_t total = Vsnprintf(_out, INT32_MAX, _format, argList);
        va_end(argList);
        return total;
    }
    
    static const char s_units[] = { 'B', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y' };
    
    template<uint32_t Kilo, char KiloCh0, char KiloCh1, CharFn fn>
    inline int32_t Prettify(char* _out, int32_t _count, uint64_t _value)
    {
        uint8_t idx = 0;
        double value = double(_value);
        while (_value != (_value & 0x7ff)
               &&     idx < COUNTOF(s_units) )
        {
            _value /= Kilo;
            value  *= 1.0/double(Kilo);
            ++idx;
        }
        
        return snprintf(_out, _count, "%0.2f %c%c%c", value
                        , fn(s_units[idx])
                        , idx > 0 ? KiloCh0 : '\0'
                        , KiloCh1
                        );
    }
    
    int32_t Prettify(char* _out, int32_t _count, uint64_t _value, Units::Enum _units)
    {
        if (Units::Kilo == _units)
        {
            return Prettify<1000, 'B', '\0', ToNoop>(_out, _count, _value);
        }
        
        return Prettify<1024, 'i', 'B', ToUpper>(_out, _count, _value);
    }
}


