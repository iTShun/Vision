#pragma once

#include "Types.h"
#include "Allocator.h"
#include <string>

namespace Utility
{
    /** @addtogroup String
     *  @{
     */
    
    /** Basic string that uses Banshee memory allocators. */
    template <typename T>
    using BasicString = std::basic_string < T, std::char_traits<T>, StdAlloc<T> >;
    
    /**    Basic string stream that uses Banshee memory allocators. */
    template <typename T>
    using BasicStringStream = std::basic_stringstream < T, std::char_traits<T>, StdAlloc<T> >;
    
    /** Wide string used primarily for handling Unicode text (UTF-32 on Linux, UTF-16 on Windows, generally). */
    using WString = BasicString<wchar_t>;
    
    /** Narrow string used for handling narrow encoded text (either locale specific ANSI or UTF-8). */
    using String = BasicString<char>;
    
    /** Wide string used UTF-16 encoded strings. */
    using U16String = BasicString<char16_t>;
    
    /** Wide string used UTF-32 encoded strings. */
    using U32String = BasicString<char32_t>;
    
    /** Wide string stream used for primarily for constructing wide strings. */
    using WStringStream = BasicStringStream<wchar_t>;
    
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
    
    /** @addtogroup String-Internal
     *  @{
     */
    
    /** Helper class used for string formatting operations. */
    class StringFormat
    {
    private:
        /**
         * Data structure used during string formatting. It holds information about parameter identifiers to replace with
         * actual parameters.
         */
        struct FormatParamRange
        {
            FormatParamRange() = default;
            FormatParamRange(UINT32 start, UINT32 identifierSize, UINT32 paramIdx)
            :start(start), identifierSize(identifierSize), paramIdx(paramIdx)
            { }
            
            UINT32 start = 0;
            UINT32 identifierSize = 0;
            UINT32 paramIdx = 0;
        };
        
        /** Structure that holds value of a parameter during string formatting. */
        template<class T>
        struct ParamData
        {
            T* buffer = nullptr;
            UINT32 size = 0;
        };
        
    public:
        /**
         * Formats the provided string by replacing the identifiers with the provided parameters. The identifiers are
         * represented like "{0}, {1}" in the source string, where the number represents the position of the parameter
         * that will be used for replacing the identifier.
         *
         * @note
         * You may use "\" to escape identifier brackets.
         * @note
         * Maximum identifier number is 19 (for a total of 20 unique identifiers. for example {20} won't be recognized as
         * an identifier).
         * @note
         * Total number of parameters that can be referenced is 200.
         */
        template<class T, class... Args>
        static BasicString<T> format(const T* source, Args&& ...args)
        {
            UINT32 strLength = getLength(source);
            
            ParamData<T> parameters[MAX_PARAMS];
            memset(parameters, 0, sizeof(parameters));
            getParams(parameters, 0U, std::forward<Args>(args)...);
            
            T bracketChars[MAX_IDENTIFIER_SIZE + 1];
            UINT32 bracketWriteIdx = 0;
            
            FormatParamRange paramRanges[MAX_PARAM_REFERENCES];
            memset(paramRanges, 0, sizeof(paramRanges));
            UINT32 paramRangeWriteIdx = 0;
            
            // Determine parameter positions
            INT32 lastBracket = -1;
            bool escaped = false;
            UINT32 charWriteIdx = 0;
            for (UINT32 i = 0; i < strLength; i++)
            {
                if (source[i] == '\\' && !escaped && paramRangeWriteIdx < MAX_PARAM_REFERENCES)
                {
                    escaped = true;
                    paramRanges[paramRangeWriteIdx++] = FormatParamRange(charWriteIdx, 1, (UINT32)-1);
                    continue;
                }
                
                if (lastBracket == -1)
                {
                    // If current char is non-escaped opening bracket start parameter definition
                    if (source[i] == '{' && !escaped)
                        lastBracket = i;
                    else
                        charWriteIdx++;
                }
                else
                {
                    if (isdigit(source[i]) && bracketWriteIdx < MAX_IDENTIFIER_SIZE)
                        bracketChars[bracketWriteIdx++] = source[i];
                    else
                    {
                        // If current char is non-escaped closing bracket end parameter definition
                        UINT32 numParamChars = bracketWriteIdx;
                        bool processedBracket = false;
                        if (source[i] == '}' && numParamChars > 0 && !escaped)
                        {
                            bracketChars[bracketWriteIdx] = '\0';
                            UINT32 paramIdx = strToInt(bracketChars);
                            if (paramIdx < MAX_PARAMS && paramRangeWriteIdx < MAX_PARAM_REFERENCES) // Check if exceeded maximum parameter limit
                            {
                                paramRanges[paramRangeWriteIdx++] = FormatParamRange(charWriteIdx, numParamChars + 2, paramIdx);
                                charWriteIdx += parameters[paramIdx].size;
                                
                                processedBracket = true;
                            }
                        }
                        
                        if (!processedBracket)
                        {
                            // Last bracket wasn't really a parameter
                            for (UINT32 j = lastBracket; j <= i; j++)
                                charWriteIdx++;
                        }
                        
                        lastBracket = -1;
                        bracketWriteIdx = 0;
                    }
                }
                
                escaped = false;
            }
            
            // Copy the clean string into output buffer
            UINT32 finalStringSize = charWriteIdx;
            
            T* outputBuffer = (T*)Alloc(finalStringSize * sizeof(T));
            UINT32 copySourceIdx = 0;
            UINT32 copyDestIdx = 0;
            for (UINT32 i = 0; i < paramRangeWriteIdx; i++)
            {
                const FormatParamRange& rangeInfo = paramRanges[i];
                UINT32 copySize = rangeInfo.start - copyDestIdx;
                
                memcpy(outputBuffer + copyDestIdx, source + copySourceIdx, copySize * sizeof(T));
                copySourceIdx += copySize + rangeInfo.identifierSize;
                copyDestIdx += copySize;
                
                if (rangeInfo.paramIdx == (UINT32)-1)
                    continue;
                
                UINT32 paramSize = parameters[rangeInfo.paramIdx].size;
                memcpy(outputBuffer + copyDestIdx, parameters[rangeInfo.paramIdx].buffer, paramSize * sizeof(T));
                copyDestIdx += paramSize;
            }
            
            memcpy(outputBuffer + copyDestIdx, source + copySourceIdx, (finalStringSize - copyDestIdx) * sizeof(T));
            
            BasicString<T> outputStr(outputBuffer, finalStringSize);
            Free(outputBuffer);
            
            for (UINT32 i = 0; i < MAX_PARAMS; i++)
            {
                if (parameters[i].buffer != nullptr)
                    Free(parameters[i].buffer);
            }
            
            return outputStr;
        }
        
    private:
        /**
         * Set of methods that can be specialized so we have a generalized way for retrieving length of strings of
         * different types.
         */
        static UINT32 getLength(const char* source) { return (UINT32)strlen(source); }
        
        /**
         * Set of methods that can be specialized so we have a generalized way for retrieving length of strings of
         * different types.
         */
        static UINT32 getLength(const wchar_t* source) { return (UINT32)wcslen(source); }
        
        /** Parses the string and returns an integer value extracted from string characters. */
        static UINT32 strToInt(const char* buffer)
        {
            return (UINT32)strtoul(buffer, nullptr, 10);
        }
        
        /** Parses the string and returns an integer value extracted from string characters. */
        static UINT32 strToInt(const wchar_t* buffer)
        {
            return (UINT32)wcstoul(buffer, nullptr, 10);
        }
        
        /**    Helper method for converting any data type to a narrow string. */
        template<class T> static std::string toString(const T& param) { return std::to_string(param); }
        
        /**    Helper method that "converts" a narrow string to a narrow string (simply a pass through). */
        static std::string toString(const std::string& param) { return param; }
        
        /**    Helper method that converts a Banshee narrow string to a standard narrow string. */
        static std::string toString(const String& param)
        {
            return std::string(param.c_str());
        }
        
        /**    Helper method that converts a narrow character array to a narrow string. */
        template<class T> static std::string toString(T* param)
        {
            static_assert(!std::is_same<T, T>::value, "Invalid pointer type.");
            return "";
        }
        
        /**    Helper method that converts a narrow character array to a narrow string. */
        static std::string toString(const char* param)
        {
            if (param == nullptr)
                return std::string();
            
            return std::string(param);
        }
        
        /**    Helper method that converts a narrow character array to a narrow string. */
        static std::string toString(char* param)
        {
            if (param == nullptr)
                return std::string();
            
            return std::string(param);
        }
        
        /**    Helper method for converting any data type to a wide string. */
        template<class T> static std::wstring toWString(const T& param) { return std::to_wstring(param); }
        
        /**    Helper method that "converts" a wide string to a wide string (simply a pass through). */
        static std::wstring toWString(const std::wstring& param) { return param; }
        
        /**    Helper method that converts a Banshee wide string to a standard wide string. */
        static std::wstring toWString(const WString& param)
        {
            return std::wstring(param.c_str());
        }
        
        /**    Helper method that converts a wide character array to a wide string. */
        template<class T> static std::wstring toWString(T* param)
        {
            static_assert(!std::is_same<T, T>::value, "Invalid pointer type.");
            return L"";
        }
        
        /**    Helper method that converts a wide character array to a wide string. */
        static std::wstring toWString(const wchar_t* param)
        {
            if (param == nullptr)
                return std::wstring();
            
            return std::wstring(param);
        }
        
        /**    Helper method that converts a wide character array to a wide string. */
        static std::wstring toWString(wchar_t* param)
        {
            if (param == nullptr)
                return std::wstring();
            
            return std::wstring(param);
        }
        
        /**
         * Converts all the provided parameters into string representations and populates the provided @p parameters array.
         */
        template<class P, class... Args>
        static void getParams(ParamData<char>* parameters, UINT32 idx, P&& param, Args&& ...args)
        {
            if (idx >= MAX_PARAMS)
                return;
            
            std::basic_string<char> sourceParam = toString(param);
            parameters[idx].buffer = (char*)Alloc((UINT32)sourceParam.size() * sizeof(char));
            parameters[idx].size = (UINT32)sourceParam.size();
            
            sourceParam.copy(parameters[idx].buffer, parameters[idx].size, 0);
            
            getParams(parameters, idx + 1, std::forward<Args>(args)...);
        }
        
        /**
         * Converts all the provided parameters into string representations and populates the provided @p parameters array.
         */
        template<class P, class... Args>
        static void getParams(ParamData<wchar_t>* parameters, UINT32 idx, P&& param, Args&& ...args)
        {
            if (idx >= MAX_PARAMS)
                return;
            
            std::basic_string<wchar_t> sourceParam = toWString(param);
            parameters[idx].buffer = (wchar_t*)Alloc((UINT32)sourceParam.size() * sizeof(wchar_t));
            parameters[idx].size = (UINT32)sourceParam.size();
            
            sourceParam.copy(parameters[idx].buffer, parameters[idx].size, 0);
            
            getParams(parameters, idx + 1, std::forward<Args>(args)...);
        }
        
        /** Helper method for parameter size calculation. Used as a stopping point in template recursion. */
        static void getParams(ParamData<char>* parameters, UINT32 idx)
        {
            // Do nothing
        }
        
        /**    Helper method for parameter size calculation. Used as a stopping point in template recursion. */
        static void getParams(ParamData<wchar_t>* parameters, UINT32 idx)
        {
            // Do nothing
        }
        
        static constexpr const UINT32 MAX_PARAMS = 20;
        static constexpr const UINT32 MAX_IDENTIFIER_SIZE = 2;
        static constexpr const UINT32 MAX_PARAM_REFERENCES = 200;
    };
    
    /** @} */
    
    /** Utility class for manipulating Strings. */
    class UTILITY_EXPORT StringUtil
    {
    public:
        /** Removes any whitespace characters from beginning or end of the string. */
        static void trim(String& str, bool left = true, bool right = true);
        
        /** @copydoc StringUtil::trim(String&, bool, bool) */
        static void trim(WString& str, bool left = true, bool right = true);
        
        /**    Removes specified characters from beginning or end of the string. */
        static void trim(String& str, const String& delims, bool left = true, bool right = true);
        
        /** @copydoc StringUtil::trim(String&, const String&, bool, bool) */
        static void trim(WString& str, const WString& delims, bool left = true, bool right = true);
        
        /**
         * Returns a vector of strings containing all the substrings delimited by the provided delimiter characters.
         *
         * @param[in]    str             The string to split.
         * @param[in]    delims       (optional) Delimiter characters to split the string by. They will not
         *                             be included in resulting substrings.
         * @param[in]    maxSplits    (optional) The maximum number of splits to perform (0 for unlimited splits). If this
         *                            parameters is > 0, the splitting process will stop after this many splits, left to right.
         */
        static Vector<String> split(const String& str, const String& delims = "\t\n ", unsigned int maxSplits = 0);
        
        /** @copydoc StringUtil::split(const String&, const String&, unsigned int) */
        static Vector<WString> split(const WString& str, const WString& delims = L"\t\n ", unsigned int maxSplits = 0);
        
        /**
         * Returns a vector of strings containing all the substrings delimited by the provided delimiter characters, or the
         * double delimiters used for including normal delimiter characters in the tokenized string.
         *
         * @param[in]    str                 The string to split.
         * @param[in]    delims           (optional) Delimiter characters to split the string by. They will not
         *                                 be included in resulting substrings.
         * @param[in]    doubleDelims    (optional) Delimiter character you may use to surround other normal delimiters,
         *                                in order to include them in the tokensized string.
         * @param[in]    maxSplits        (optional) The maximum number of splits to perform (0 for unlimited splits).
         *                                If this parameters is > 0, the splitting process will stop after this many splits,
         *                                left to right.
         */
        static Vector<String> tokenise(const String& str, const String& delims = "\t\n ",
                                       const String& doubleDelims = "\"", unsigned int maxSplits = 0);
        
        /** @copydoc StringUtil::tokenise(const String&, const String&, const String&, unsigned int) */
        static Vector<WString> tokenise(const WString& str, const WString& delims = L"\t\n ",
                                        const WString& doubleDelims = L"\"", unsigned int maxSplits = 0);
        
        /** Converts all the characters in the string to lower case. Does not handle UTF8 encoded strings. */
        static void toLowerCase(String& str);
        
        /** Converts all the characters in the string to lower case. Does not handle UTF8 encoded strings. */
        static void toLowerCase(WString& str);
        
        /** Converts all the characters in the string to upper case. Does not handle UTF8 encoded strings. */
        static void toUpperCase(String& str);
        
        /**    Converts all the characters in the string to upper case. Does not handle UTF8 encoded strings. */
        static void toUpperCase(WString& str);
        
        /**
         * Returns whether the string begins with the pattern passed in.
         *
         * @param[in]    str             String to compare.
         * @param[in]    pattern        Pattern to compare with.
         * @param[in]    lowerCase    (optional) If true, the start of the string will be lower cased before comparison, and
         *                            the pattern should also be in lower case.
         */
        static bool startsWith(const String& str, const String& pattern, bool lowerCase = true);
        
        /** @copydoc startsWith(const String&, const String&, bool) */
        static bool startsWith(const WString& str, const WString& pattern, bool lowerCase = true);
        
        /**
         * Returns whether the string end with the pattern passed in.
         *
         * @param[in]    str             String to compare.
         * @param[in]    pattern        Pattern to compare with.
         * @param[in]    lowerCase    (optional) If true, the start of the string will be lower cased before comparison, and
         *                            the pattern should also be in lower case.
         */
        static bool endsWith(const String& str, const String& pattern, bool lowerCase = true);
        
        /** @copydoc endsWith(const String&, const String&, bool) */
        static bool endsWith(const WString& str, const WString& pattern, bool lowerCase = true);
        
        /**
         * Returns true if the string matches the provided pattern. Pattern may use a "*" wildcard for matching any
         * characters.
         *
         * @param[in]    str                 The string to test.
         * @param[in]    pattern             Patterns to look for.
         * @param[in]    caseSensitive    (optional) Should the match be case sensitive or not.
         */
        static bool match(const String& str, const String& pattern, bool caseSensitive = true);
        
        /** @copydoc match(const String&, const String&, bool) */
        static bool match(const WString& str, const WString& pattern, bool caseSensitive = true);
        
        /**
         * Replace all instances of a substring with a another substring.
         *
         * @param[in]    source               String to search.
         * @param[in]    replaceWhat           Substring to find and replace
         * @param[in]    replaceWithWhat    Substring to replace with (the new sub-string)
         *
         * @return    An updated string with the substrings replaced.
         */
        static const String replaceAll(const String& source, const String& replaceWhat, const String& replaceWithWhat);
        
        /** @copydoc replaceAll(const String&, const String&, const String&) */
        static const WString replaceAll(const WString& source, const WString& replaceWhat, const WString& replaceWithWhat);
        
        /**
         * Compares two strings. Returns 0 if the two compare equal, <0 if the value of the left string is lower than of
         * the right string, or >0 if the value of the left string is higher than the right string.
         *
         * @param[in]    lhs                Left string to compare.
         * @param[in]    rhs                Right string to compare.
         * @param[in]    caseSensitive    If true the comparison will consider uppercase and lowercase characters different.
         *                                Note that case conversion does not handle UTF8 strings.
         */
        template <class T>
        static int compare(const BasicString<T>& lhs, const BasicString<T>& rhs, bool caseSensitive = true)
        {
            if (caseSensitive)
                return (int)lhs.compare(rhs);
            
            int size = (int)std::min(lhs.size(), rhs.size());
            for (int i = 0; i < size; i++)
            {
                if (toupper(lhs[i]) < toupper(rhs[i])) return -1;
                if (toupper(lhs[i]) > toupper(rhs[i])) return 1;
            }
            
            return (lhs.size() < rhs.size() ? -1 : (lhs.size() == rhs.size() ? 0 : 1));
        }
        
        /** @copydoc StringFormat::format */
        template<class T, class... Args>
        static BasicString<T> format(const BasicString<T>& source, Args&& ...args)
        {
            return StringFormat::format(source.c_str(), std::forward<Args>(args)...);
        }
        
        /** @copydoc StringFormat::format */
        template<class T, class... Args>
        static BasicString<T> format(const T* source, Args&& ...args)
        {
            return StringFormat::format(source, std::forward<Args>(args)...);
        }
        
        /** Constant blank string, useful for returning by ref where local does not exist. */
        static const String BLANK;
        
        /**    Constant blank wide string, useful for returning by ref where local does not exist. */
        static const WString WBLANK;
        
    private:
        template <class T>
        static Vector<BasicString<T>> splitInternal(const BasicString<T>& str, const BasicString<T>& delims, unsigned int maxSplits)
        {
            Vector<BasicString<T>> ret;
            // Pre-allocate some space for performance
            ret.reserve(maxSplits ? maxSplits + 1 : 10);    // 10 is guessed capacity for most case
            
            unsigned int numSplits = 0;
            
            // Use STL methods
            size_t start, pos;
            start = 0;
            do
            {
                pos = str.find_first_of(delims, start);
                if (pos == start)
                {
                    // Do nothing
                    start = pos + 1;
                }
                else if (pos == BasicString<T>::npos || (maxSplits && numSplits == maxSplits))
                {
                    // Copy the rest of the string
                    ret.push_back(str.substr(start));
                    break;
                }
                else
                {
                    // Copy up to delimiter
                    ret.push_back(str.substr(start, pos - start));
                    start = pos + 1;
                }
                // parse up to next real data
                start = str.find_first_not_of(delims, start);
                ++numSplits;
                
            } while (pos != BasicString<T>::npos);
            
            return ret;
        }
        
        template <class T>
        static Vector<BasicString<T>> tokeniseInternal(const BasicString<T>& str, const BasicString<T>& singleDelims,
                                                       const BasicString<T>& doubleDelims, unsigned int maxSplits)
        {
            Vector<BasicString<T>> ret;
            // Pre-allocate some space for performance
            ret.reserve(maxSplits ? maxSplits + 1 : 10);    // 10 is guessed capacity for most case
            
            unsigned int numSplits = 0;
            BasicString<T> delims = singleDelims + doubleDelims;
            
            // Use STL methods
            size_t start, pos;
            T curDoubleDelim = 0;
            start = 0;
            do
            {
                if (curDoubleDelim != 0)
                {
                    pos = str.find(curDoubleDelim, start);
                }
                else
                {
                    pos = str.find_first_of(delims, start);
                }
                
                if (pos == start)
                {
                    T curDelim = str.at(pos);
                    if (doubleDelims.find_first_of(curDelim) != BasicString<T>::npos)
                    {
                        curDoubleDelim = curDelim;
                    }
                    // Do nothing
                    start = pos + 1;
                }
                else if (pos == BasicString<T>::npos || (maxSplits && numSplits == maxSplits))
                {
                    if (curDoubleDelim != 0)
                    {
                        //Missing closer. Warn or throw exception?
                    }
                    // Copy the rest of the string
                    ret.push_back(str.substr(start));
                    break;
                }
                else
                {
                    if (curDoubleDelim != 0)
                    {
                        curDoubleDelim = 0;
                    }
                    
                    // Copy up to delimiter
                    ret.push_back(str.substr(start, pos - start));
                    start = pos + 1;
                }
                if (curDoubleDelim == 0)
                {
                    // parse up to next real data
                    start = str.find_first_not_of(singleDelims, start);
                }
                
                ++numSplits;
                
            } while (pos != BasicString<T>::npos);
            
            return ret;
        }
        
        template <class T>
        static bool startsWithInternal(const BasicString<T>& str, const BasicString<T>& pattern, bool lowerCase)
        {
            size_t thisLen = str.length();
            size_t patternLen = pattern.length();
            if (thisLen < patternLen || patternLen == 0)
                return false;
            
            BasicString<T> startOfThis = str.substr(0, patternLen);
            if (lowerCase)
                StringUtil::toLowerCase(startOfThis);
            
            return (startOfThis == pattern);
        }
        
        template <class T>
        static bool endsWithInternal(const BasicString<T>& str, const BasicString<T>& pattern, bool lowerCase)
        {
            size_t thisLen = str.length();
            size_t patternLen = pattern.length();
            if (thisLen < patternLen || patternLen == 0)
                return false;
            
            BasicString<T> endOfThis = str.substr(thisLen - patternLen, patternLen);
            if (lowerCase)
                StringUtil::toLowerCase(endOfThis);
            
            return (endOfThis == pattern);
        }
        
        template <class T>
        static bool matchInternal(const BasicString<T>& str, const BasicString<T>& pattern, bool caseSensitive)
        {
            BasicString<T> tmpStr = str;
            BasicString<T> tmpPattern = pattern;
            if (!caseSensitive)
            {
                StringUtil::toLowerCase(tmpStr);
                StringUtil::toLowerCase(tmpPattern);
            }
            
            typename BasicString<T>::const_iterator strIt = tmpStr.begin();
            typename BasicString<T>::const_iterator patIt = tmpPattern.begin();
            typename BasicString<T>::const_iterator lastWildCardIt = tmpPattern.end();
            while (strIt != tmpStr.end() && patIt != tmpPattern.end())
            {
                if (*patIt == '*')
                {
                    lastWildCardIt = patIt;
                    // Skip over looking for next character
                    ++patIt;
                    if (patIt == tmpPattern.end())
                    {
                        // Skip right to the end since * matches the entire rest of the string
                        strIt = tmpStr.end();
                    }
                    else
                    {
                        // scan until we find next pattern character
                        while (strIt != tmpStr.end() && *strIt != *patIt)
                            ++strIt;
                    }
                }
                else
                {
                    if (*patIt != *strIt)
                    {
                        if (lastWildCardIt != tmpPattern.end())
                        {
                            // The last wildcard can match this incorrect sequence
                            // rewind pattern to wildcard and keep searching
                            patIt = lastWildCardIt;
                            lastWildCardIt = tmpPattern.end();
                        }
                        else
                        {
                            // no wildwards left
                            return false;
                        }
                    }
                    else
                    {
                        ++patIt;
                        ++strIt;
                    }
                }
                
            }
            
            // If we reached the end of both the pattern and the string, we succeeded
            if (patIt == tmpPattern.end() && strIt == tmpStr.end())
                return true;
            else
                return false;
        }
        
        template <class T>
        static BasicString<T> replaceAllInternal(const BasicString<T>& source,
                                                 const BasicString<T>& replaceWhat, const BasicString<T>& replaceWithWhat)
        {
            BasicString<T> result = source;
            typename BasicString<T>::size_type pos = 0;
            while (1)
            {
                pos = result.find(replaceWhat, pos);
                if (pos == BasicString<T>::npos) break;
                result.replace(pos, replaceWhat.size(), replaceWithWhat);
                pos += replaceWithWhat.size();
            }
            return result;
        }
    };
    
    /************************************************************************/
    /*                         VARIOUS TO STRING CONVERSIONS                   */
    /************************************************************************/
    
    /** Converts a narrow string to a wide string. */
    UTILITY_EXPORT WString toWString(const String& source);
    
    /**    Converts a narrow string to a wide string. */
    UTILITY_EXPORT WString toWString(const char* source);
    
    /** Converts a float to a string. */
    UTILITY_EXPORT WString toWString(float val, unsigned short precision = 6,
                                     unsigned short width = 0, char fill = ' ',
                                     std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /** Converts a double to a string. */
    UTILITY_EXPORT WString toWString(double val, unsigned short precision = 6,
                                     unsigned short width = 0, char fill = ' ',
                                     std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts an int to a string. */
    UTILITY_EXPORT WString toWString(int val, unsigned short width = 0,
                                     char fill = ' ',
                                     std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts an unsigned int to a string. */
    UTILITY_EXPORT WString toWString(unsigned int val,
                                     unsigned short width = 0, char fill = ' ',
                                     std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts an 64bit integer to a string. */
    UTILITY_EXPORT WString toWString(INT64 val,
                                     unsigned short width = 0, char fill = ' ',
                                     std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts an 64bit unsigned to a string. */
    UTILITY_EXPORT WString toWString(UINT64 val,
                                     unsigned short width = 0, char fill = ' ',
                                     std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts an narrow char unsigned to a string. */
    UTILITY_EXPORT WString toWString(char val,
                                     unsigned short width = 0, char fill = ' ',
                                     std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts an wide bit char unsigned to a string. */
    UTILITY_EXPORT WString toWString(wchar_t val,
                                     unsigned short width = 0, char fill = ' ',
                                     std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**
     * Converts a boolean to a string.
     *
     * @param[in]    val      Value to convert.
     * @param[in]    yesNo    (optional) If set to true, result is "yes" or "no" instead of "true" or "false".
     */
    UTILITY_EXPORT WString toWString(bool val, bool yesNo = false);
    
    /** Converts a vector of strings into a single string where the substrings are delimited by spaces. */
    UTILITY_EXPORT WString toWString(const Vector<WString>& val);
    
    /** Converts a wide string to a narrow string. */
    UTILITY_EXPORT String toString(const WString& source);
    
    /**    Converts a wide string to a narrow string. */
    UTILITY_EXPORT String toString(const wchar_t* source);
    
    /**    Converts a float to a string. */
    UTILITY_EXPORT String toString(float val, unsigned short precision = 6,
                                   unsigned short width = 0, char fill = ' ',
                                   std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts a double to a string. */
    UTILITY_EXPORT String toString(double val, unsigned short precision = 6,
                                   unsigned short width = 0, char fill = ' ',
                                   std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts an int to a string. */
    UTILITY_EXPORT String toString(int val, unsigned short width = 0,
                                   char fill = ' ',
                                   std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts an unsigned int to a string. */
    UTILITY_EXPORT String toString(unsigned int val,
                                   unsigned short width = 0, char fill = ' ',
                                   std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts a 64bit int to a string. */
    UTILITY_EXPORT String toString(INT64 val,
                                   unsigned short width = 0, char fill = ' ',
                                   std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**    Converts an 64bit unsigned int to a string. */
    UTILITY_EXPORT String toString(UINT64 val,
                                   unsigned short width = 0, char fill = ' ',
                                   std::ios::fmtflags flags = std::ios::fmtflags(0));
    
    /**
     * Converts a boolean to a string.
     *
     * @param[in]    val      true to value.
     * @param[in]    yesNo    (optional) If set to true, result is "yes" or "no" instead of "true" or "false".
     */
    UTILITY_EXPORT String toString(bool val, bool yesNo = false);
    
    /**
     * Converts a vector of strings into a single string where the substrings are delimited by spaces.
     */
    UTILITY_EXPORT String toString(const Vector<String>& val);
    
    /**
     * Converts a String to a float.
     *
     * @note    0.0f if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT float parseFloat(const String& val, float defaultValue = 0);
    
    /**
     * Converts a String to a whole number.
     *
     * @note    0 if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT INT32 parseINT32(const String& val, INT32 defaultValue = 0);
    
    /**
     * Converts a String to a whole number.
     *
     * @note    0 if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT UINT32 parseUINT32(const String& val, UINT32 defaultValue = 0);
    
    /**
     * Converts a String to a whole number.
     *
     * @note    0 if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT INT64 parseINT64(const String& val, INT64 defaultValue = 0);
    
    /**
     * Converts a String to a whole number.
     *
     * @note    0 if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT UINT64 parseUINT64(const String& val, UINT64 defaultValue = 0);
    
    /**
     * Converts a String to a boolean.
     *
     * @note    Returns true if case-insensitive match of the start of the string matches "true", "yes" or "1",
     *            false otherwise.
     */
    UTILITY_EXPORT bool parseBool(const String& val, bool defaultValue = 0);
    
    /** Checks the String is a valid number value. */
    UTILITY_EXPORT bool isNumber(const String& val);
    
    /**
     * Converts a WString to a float.
     *
     * @note    0.0f if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT float parseFloat(const WString& val, float defaultValue = 0);
    
    /**
     * Converts a WString to a whole number.
     *
     * @note    0 if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT INT32 parseINT32(const WString& val, INT32 defaultValue = 0);
    
    /**
     * Converts a WString to a whole number.
     *
     * @note    0 if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT UINT32 parseUINT32(const WString& val, UINT32 defaultValue = 0);
    
    /**
     * Converts a WString to a whole number.
     *
     * @note    0 if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT INT64 parseINT64(const WString& val, INT64 defaultValue = 0);
    
    /**
     * Converts a WString to a whole number.
     *
     * @note    0 if the value could not be parsed, otherwise the numeric version of the string.
     */
    UTILITY_EXPORT UINT64 parseUINT64(const WString& val, UINT64 defaultValue = 0);
    
    /**
     * Converts a WString to a boolean.
     *
     * @note    Returns true if case-insensitive match of the start of the string
     *            matches "true", "yes" or "1", false otherwise.
     */
    UTILITY_EXPORT bool parseBool(const WString& val, bool defaultValue = 0);
    
    /**
     * Checks the WString is a valid number value.
     */
    UTILITY_EXPORT bool isNumber(const WString& val);
    
    /** @name Internal
     *  @{
     */
    
    /** Helper method that throws an exception regarding a data overflow. */
    void UTILITY_EXPORT __string_throwDataOverflowException();
    
    /** @} */
}

/** @cond STDLIB */

namespace std
{
    /** Hash value generator for String. */
    template<>
    struct hash<Utility::String>
    {
        size_t operator()(const Utility::String& string) const
        {
            size_t hash = 0;
            for (size_t i = 0; i < string.size(); i++)
                hash = 65599 * hash + string[i];
            return hash ^ (hash >> 16);
        }
    };
    
    /**    Hash value generator for WString. */
    template<>
    struct hash<Utility::WString>
    {
        size_t operator()(const Utility::WString& string) const
        {
            size_t hash = 0;
            for (size_t i = 0; i < string.size(); i++)
                hash = 65599 * hash + string[i];
            return hash ^ (hash >> 16);
        }
    };
}

/** @endcond */
