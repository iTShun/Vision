#pragma once

#include "GenericKit/Platform.h"
#include "GenericKit/Macros.h"

// alloca
#if CRT_MSVC || CRT_MINGW
#	include <malloc.h>
#endif

#if PLATFORM_BSD
#	if defined(__GLIBC__)
#		include_next <alloca.h>
#	else
#		include <stdlib.h>
#	endif
#elif PLATFORM_IOS
#	include <malloc/malloc.h>
#elif PLATFORM_OSX
#	include <malloc/malloc.h>
#	include <alloca.h>
#endif

#include <stdarg.h> // va_list
#include <stdint.h> // uint32_t
#include <stdlib.h> // size_t
#include <stddef.h> // ptrdiff_t
#include <stdbool.h> // bool
#include <assert.h>
#include <algorithm>

///
#define COUNTOF(_x) sizeof(GenericKit::COUNTOF_REQUIRES_ARRAY_ARGUMENT(_x) )

///
#define IGNORE_C4127(_x) GenericKit::IgnoreC4127(!!(_x) )

///
#define ENABLED(_x) GenericKit::IsEnabled<!!(_x)>()

namespace GenericKit
{
	/// Template for avoiding MSVC: C4127: conditional expression is constant
	template<bool>
	constexpr bool IsEnabled();

	///
	template<class Ty>
	constexpr bool IsTriviallyCopyable();

	/// Exchange two values.
	template<typename Ty>
	void Xchg(Ty& _a, Ty& _b);

	/// Exchange memory.
	void Xchg(void* _a, void* _b, size_t _numBytes);

	/// Returns minimum of two values.
	template<typename Ty>
	constexpr Ty Min(const Ty& _a, const Ty& _b);

	/// Returns maximum of two values.
	template<typename Ty>
	constexpr Ty Max(const Ty& _a, const Ty& _b);

	/// Returns minimum of three values.
	template<typename Ty>
	constexpr Ty Min(const Ty& _a, const Ty& _b, const Ty& _c);

	/// Returns maximum of three values.
	template<typename Ty>
	constexpr Ty Max(const Ty& _a, const Ty& _b, const Ty& _c);

	/// Returns middle of three values.
	template<typename Ty>
	constexpr Ty Mid(const Ty& _a, const Ty& _b, const Ty& _c);

	/// Returns clamped value between min/max.
	template<typename Ty>
	constexpr Ty Clamp(const Ty& _a, const Ty& _min, const Ty& _max);

	/// Returns true if value is power of 2.
	template<typename Ty>
	constexpr bool IsPowerOf2(Ty _a);

	// http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
	template<typename T, size_t N>
	char(&COUNTOF_REQUIRES_ARRAY_ARGUMENT(const T(&)[N]))[N];

	///
	void MemCopy(void* _dst, const void* _src, size_t _numBytes);

	///
	void MemCopy(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch, uint32_t _dstPitch);

	///
	void Gather(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch);

	///
	void Scatter(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _dstPitch);

	///
	void MemMove(void* _dst, const void* _src, size_t _numBytes);

	///
	void MemSet(void* _dst, uint8_t _ch, size_t _numBytes);

	///
	int32_t MemCmp(const void* _lhs, const void* _rhs, size_t _numBytes);
}

#include "GenericKit/General.inl"

