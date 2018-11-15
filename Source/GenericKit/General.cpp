#include "General.h"

#if !CRT_NONE
#	include <string.h> // memcpy, memmove, memset
#endif // !CRT_NONE

namespace GenericKit
{
	void Xchg(void* _a, void* _b, size_t _numBytes)
	{
		uint8_t* lhs = (uint8_t*)_a;
		uint8_t* rhs = (uint8_t*)_b;
		const uint8_t* end = rhs + _numBytes;
		while (rhs != end)
		{
			Xchg(*lhs++, *rhs++);
		}
	}

	void MemCopyRef(void* _dst, const void* _src, size_t _numBytes)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* end = dst + _numBytes;
		const uint8_t* src = (const uint8_t*)_src;
		while (dst != end)
		{
			*dst++ = *src++;
		}
	}

	void MemCopy(void* _dst, const void* _src, size_t _numBytes)
	{
#if CRT_NONE
		memCopyRef(_dst, _src, _numBytes);
#else
		::memcpy(_dst, _src, _numBytes);
#endif // CRT_NONE
	}

	void MemCopy(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch, uint32_t _dstPitch)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t ii = 0; ii < _num; ++ii)
		{
			MemCopy(dst, src, _size);
			src += _srcPitch;
			dst += _dstPitch;
		}
	}

	///
	void Gather(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch)
	{
		MemCopy(_dst, _src, _size, _num, _srcPitch, _size);
	}

	///
	void Scatter(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _dstPitch)
	{
		MemCopy(_dst, _src, _size, _num, _size, _dstPitch);
	}

	void MemMoveRef(void* _dst, const void* _src, size_t _numBytes)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* src = (const uint8_t*)_src;

		if (_numBytes == 0
			|| dst == src)
		{
			return;
		}

		//	if (src+_numBytes <= dst || end <= src)
		if (dst < src)
		{
			MemCopy(_dst, _src, _numBytes);
			return;
		}

		for (intptr_t ii = _numBytes - 1; ii >= 0; --ii)
		{
			dst[ii] = src[ii];
		}
	}

	void MemMove(void* _dst, const void* _src, size_t _numBytes)
	{
#if CRT_NONE
		MemMoveRef(_dst, _src, _numBytes);
#else
		::memmove(_dst, _src, _numBytes);
#endif // CRT_NONE
	}

	void MemSetRef(void* _dst, uint8_t _ch, size_t _numBytes)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* end = dst + _numBytes;
		while (dst != end)
		{
			*dst++ = char(_ch);
		}
	}

	void MemSet(void* _dst, uint8_t _ch, size_t _numBytes)
	{
#if CRT_NONE
		MemSetRef(_dst, _ch, _numBytes);
#else
		::memset(_dst, _ch, _numBytes);
#endif // CRT_NONE
	}

	int32_t MemCmpRef(const void* _lhs, const void* _rhs, size_t _numBytes)
	{
		const char* lhs = (const char*)_lhs;
		const char* rhs = (const char*)_rhs;
		for (
			; 0 < _numBytes && *lhs == *rhs
			; ++lhs, ++rhs, --_numBytes
			)
		{
		}

		return 0 == _numBytes ? 0 : *lhs - *rhs;
	}

	int32_t MemCmp(const void* _lhs, const void* _rhs, size_t _numBytes)
	{
#if CRT_NONE
		return MemCmpRef(_lhs, _rhs, _numBytes);
#else
		return ::memcmp(_lhs, _rhs, _numBytes);
#endif // CRT_NONE
	}
}