#include "General/LSDynLib.h"
#include "Error/LSException.h"

#if PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#if !defined(NOMINMAX) && defined(_MSC_VER)
		#define NOMINMAX // required to stop windows.h messing up std::min
	#endif
	#include <windows.h>
#endif

#if PLATFORM_OSX
	#include <dlfcn.h>
#endif

namespace ls
{
	DynLib::DynLib(String name)
		:mName(std::move(name))
	{
		load();
	}

	void DynLib::load()
	{
		if (mHandle)
			return;

		mHandle = (DYNLIB_HANDLE)DYNLIB_LOAD(mName.c_str());

		if (!mHandle)
		{
			LS_EXCEPT(InternalErrorException,
				"Could not load dynamic library " + mName +
				".  System Error: " + dynlibError());
		}
	}

	void DynLib::unload()
	{
		if (!mHandle)
			return;

		if (DYNLIB_UNLOAD(mHandle))
		{
			LS_EXCEPT(InternalErrorException,
				"Could not unload dynamic library " + mName +
				".  System Error: " + dynlibError());
		}
	}

	void* DynLib::getSymbol(const String& strName) const
	{
		if (!mHandle)
			return nullptr;

		return (void*)DYNLIB_GETSYM(mHandle, strName.c_str());
	}

	String DynLib::dynlibError()
	{
#if PLATFORM_WINDOWS
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			NULL
		);

		String ret((char*)lpMsgBuf);

		// Free the buffer.
		LocalFree(lpMsgBuf);
		return ret;
#elif PLATFORM_LINUX || PLATFORM_OSX
		return String(dlerror());
#else
		return String();
#endif
	}
}
