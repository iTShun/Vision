/////////////////////////////////////////////////////////////////////
#include "GenericKit/General.h"

#if CRT_MSVC && !defined(va_copy)
#    define va_copy(_a, _b) (_a) = (_b)
#endif // CRT_MSVC && !defined(va_copy)

namespace GenericKit
{
    
    template<bool>
    inline constexpr bool IsEnabled()
    {
        return true;
    }

    template<class Ty>
    inline constexpr bool IsTriviallyCopyable()
    {
        return __is_trivially_copyable(Ty);
    }

    template<>
    inline constexpr bool IsEnabled<false>()
    {
        return false;
    }

    inline constexpr bool IgnoreC4127(bool _x)
    {
        return _x;
    }

    template<typename Ty>
    inline void Xchg(Ty& _a, Ty& _b)
    {
        Ty tmp = _a; _a = _b; _b = tmp;
    }

    template<typename Ty>
    inline constexpr Ty Min(const Ty& _a, const Ty& _b)
    {
        return _a < _b ? _a : _b;
    }

    template<typename Ty>
    inline constexpr Ty Max(const Ty& _a, const Ty& _b)
    {
        return _a > _b ? _a : _b;
    }

    template<typename Ty>
    inline constexpr Ty Min(const Ty& _a, const Ty& _b, const Ty& _c)
    {
        return Min(Min(_a, _b), _c);
    }

    template<typename Ty>
    inline constexpr Ty Max(const Ty& _a, const Ty& _b, const Ty& _c)
    {
        return Max(Max(_a, _b), _c);
    }

    template<typename Ty>
    inline constexpr Ty Mid(const Ty& _a, const Ty& _b, const Ty& _c)
    {
        return Max(Min(_a, _b), Min(Max(_a, _b), _c));
    }

    template<typename Ty>
    inline constexpr Ty Clamp(const Ty& _a, const Ty& _min, const Ty& _max)
    {
        return Max(Min(_a, _max), _min);
    }

    template<typename Ty>
    inline constexpr bool IsPowerOf2(Ty _a)
    {
        return _a && !(_a & (_a - 1));
    }
    
}
