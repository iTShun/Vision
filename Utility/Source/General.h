#pragma once

#if PLATFORM_WINDOWS
#	include <Windows.h>
#endif

namespace Utility
{
	/** @addtogroup General
	 *  @{
	 */

	 /** Generates a new hash for the provided type using the default standard hasher and combines it with a previous hash. */
	template <class T>
	void hash_combine(std::size_t& seed, const T& v)
	{
		using HashType = typename std::conditional<std::is_enum<T>::value, EnumClassHash, std::hash<T>>::type;

		HashType hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	void terminate(bool force)
	{
#if PLATFORM_WINDOWS
		if (!force)
			PostQuitMessage(0);
		else
			TerminateProcess(GetCurrentProcess(), 0);
#endif
	}

	/** @} */
}