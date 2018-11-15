#pragma once

#include "GenericKit/General.h"

#if CRT_MSVC && !defined(va_copy)
#    define va_copy(_a, _b) (_a) = (_b)
#endif // CRT_MSVC && !defined(va_copy)

namespace GenericKit
{    
    /// Units
    struct Units
    {
        enum Enum //!< Units:
        {
            Kilo, //!< SI units
            Kibi, //!< IEC prefix
        };
    };

    /// Retruns true if character is part of space set.
    bool IsSpace(char _ch);

    /// Retruns true if character is uppercase.
    bool IsUpper(char _ch);

    /// Retruns true if character is lowercase.
    bool IsLower(char _ch);

    /// Returns true if character is part of alphabet set.
    bool IsAlpha(char _ch);

    /// Returns true if character is part of numeric set.
    bool IsNumeric(char _ch);

    /// Returns true if character is part of alpha numeric set.
    bool IsAlphaNum(char _ch);

    /// Returns true if character is part of hexadecimal set.
    bool IsHexNum(char _ch);

    /// Returns true if character is printable.
    bool IsPrint(char _ch);

    /// Retruns lower case character representing _ch.
    char ToLower(char _ch);

    /// Lower case string in place assuming length passed is valid.
    void ToLowerUnsafe(char* _inOutStr, int32_t _len);

    /// Lower case string in place.
    void ToLower(char* _inOutStr, int32_t _max = INT32_MAX);

    /// Returns upper case character representing _ch.
    char ToUpper(char _ch);

    /// Upper case string in place assuming length passed is valid.
    void ToUpperUnsafe(char* _inOutStr, int32_t _len);

    /// Uppre case string in place.
    void ToUpper(char* _inOutStr, int32_t _max = INT32_MAX);

    /// Get string length.
    int32_t StrLen(const char* _str, int32_t _max = INT32_MAX);

    /// Skip whitespace.
    const char* Strws(const char* _str);

    /// Skip non-whitespace.
    const char* Strnws(const char* _str);

    /// Returns pointer to first character after word.
    const char* StrSkipWord(const char* _str, int32_t _max = INT32_MAX);

    /// Find matching block.
    const char* Strmb(const char* _str, char _open, char _close);

    // Normalize string to sane line endings.
    void EolLF(char* _out, int32_t _size, const char* _str);

    /// Cross platform implementation of vsnprintf that returns number of
    /// characters which would have been written to the final string if
    /// enough space had been available.
    int32_t Vsnprintf(char* _out, int32_t _max, const char* _format, va_list _argList);

    /// Cross platform implementation of snprintf that returns number of
    /// characters which would have been written to the final string if
    /// enough space had been available.
    int32_t Snprintf(char* _out, int32_t _max, const char* _format, ...);

    int32_t Sprintf(char* _out, const char* _format, ...);
    
    /// Convert size in bytes to human readable string kibi units.
    int32_t Prettify(char* _out, int32_t _count, uint64_t _value, Units::Enum _units = Units::Kibi);

    /// Converts bool value to string.
    int32_t ToString(char* _out, int32_t _max, bool _value);

    /// Converts double value to string.
    int32_t ToString(char* _out, int32_t _max, double _value);

    /// Converts 32-bit integer value to string.
    int32_t ToString(char* _out, int32_t _max, int32_t _value, uint32_t _base = 10);

    /// Converts 64-bit integer value to string.
    int32_t ToString(char* _out, int32_t _max, int64_t _value, uint32_t _base = 10);

    /// Converts 32-bit unsigned integer value to string.
    int32_t ToString(char* _out, int32_t _max, uint32_t _value, uint32_t _base = 10);

    /// Converts 64-bit unsigned integer value to string.
    int32_t ToString(char* _out, int32_t _max, uint64_t _value, uint32_t _base = 10);
}
