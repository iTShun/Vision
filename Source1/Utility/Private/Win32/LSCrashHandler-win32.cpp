#include "Prerequisites/LSPrerequisitesUtil.h"

#if PLATFORM_WINDOWS

#include "Logger/LSLogger.h"
#include "General/LSDynLib.h"
#include "FileSystem/LSFileSystem.h"


#include "windows.h"
#include <psapi.h>

// Disable warning in VS2015 that's not under my control
#pragma warning(disable : 4091)
#include "DbgHelp.h"
#include "String/LSUnicode.h"
#pragma warning(default : 4091)

#pragma comment(lib, "DbgHelp.lib")

static const char* sMiniDumpName = "minidump.dmp";

namespace ls
{
	CrashHandler::CrashHandler()
	{
		m = ls_new<Data>();
	}

	CrashHandler::~CrashHandler()
	{
		ls_delete(m);
	}

	/**
	 * Returns the raw stack trace using the provided context. Raw stack trace contains only function addresses.
	 * 			
	 * @param[in]	context		Processor context from which to start the stack trace. 
	 * @param[in]	stackTrace	Output parameter that will contain the function addresses. First address is the deepest
	 * 							called function and following address is its caller and so on.
	 * @return					Number of functions in the call stack.
	 */
	UINT32 win32_getRawStackTrace(CONTEXT context, UINT64 stackTrace[LS_MAX_STACKTRACE_DEPTH])
	{
		HANDLE hProcess = GetCurrentProcess();
		HANDLE hThread = GetCurrentThread();
		UINT32 machineType;

		STACKFRAME64 stackFrame;
		memset(&stackFrame, 0, sizeof(stackFrame));

		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrStack.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Mode = AddrModeFlat;

#if ARCH_64BIT
		stackFrame.AddrPC.Offset = context.Rip;
		stackFrame.AddrStack.Offset = context.Rsp;
		stackFrame.AddrFrame.Offset = context.Rbp;

		machineType = IMAGE_FILE_MACHINE_AMD64;
#else
		stackFrame.AddrPC.Offset = context.Eip;
		stackFrame.AddrStack.Offset = context.Esp;
		stackFrame.AddrFrame.Offset = context.Ebp;

		machineType = IMAGE_FILE_MACHINE_I386;
#endif

		UINT32 numEntries = 0;
		while (true)
		{
			if (!StackWalk64(machineType, hProcess, hThread, &stackFrame, &context, nullptr,
				SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
			{
				break;
			}

			if (numEntries < LS_MAX_STACKTRACE_DEPTH)
				stackTrace[numEntries] = stackFrame.AddrPC.Offset;

			numEntries++;

			if (stackFrame.AddrPC.Offset == 0 || stackFrame.AddrFrame.Offset == 0)
				break;
		}

		return numEntries;
	}

	/**
	 * Returns a string containing a stack trace using the provided context. If function can be found in the symbol table
	 * its readable name will be present in the stack trace, otherwise just its address.
	 * 			
	 * @param[in]	context		Processor context from which to start the stack trace. 
	 * @param[in]	skip		Number of bottom-most call stack entries to skip.
	 * @return					String containing the call stack with each function on its own line.
	 */
	String win32_getStackTrace(CONTEXT context, UINT32 skip = 0)
	{
		UINT64 rawStackTrace[LS_MAX_STACKTRACE_DEPTH];
		UINT32 numEntries = win32_getRawStackTrace(context, rawStackTrace);

		numEntries = std::min((UINT32)LS_MAX_STACKTRACE_DEPTH, numEntries);

		UINT32 bufferSize = sizeof(PIMAGEHLP_SYMBOL64) + LS_MAX_STACKTRACE_NAME_BYTES;
		UINT8* buffer = (UINT8*)ls_alloc(bufferSize);

		PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)buffer;
		symbol->SizeOfStruct = bufferSize;
		symbol->MaxNameLength = LS_MAX_STACKTRACE_NAME_BYTES;

		HANDLE hProcess = GetCurrentProcess();

		StringStream outputStream;
		for (UINT32 i = skip; i < numEntries; i++)
		{
			if (i > skip)
				outputStream << std::endl;

			DWORD64 funcAddress = rawStackTrace[i];

			// Output function name
			DWORD64 dummy;
			if (SymGetSymFromAddr64(hProcess, funcAddress, &dummy, symbol))
				outputStream << StringUtil::format("{0}() - ", symbol->Name);

			// Output file name and line
			IMAGEHLP_LINE64	lineData;
			lineData.SizeOfStruct = sizeof(lineData);

			String addressString = toString(funcAddress, 0, ' ', std::ios::hex);

			DWORD column;
			if (SymGetLineFromAddr64(hProcess, funcAddress, &column, &lineData))
			{
				Path filePath = lineData.FileName;

				outputStream << StringUtil::format("0x{0} File[{1}:{2} ({3})]", addressString, 
					filePath.getFilename(), lineData.LineNumber, column);
			}
			else
			{
				outputStream << StringUtil::format("0x{0}", addressString);
			}

			// Output module name
			IMAGEHLP_MODULE64 moduleData;
			moduleData.SizeOfStruct = sizeof(moduleData);

			if (SymGetModuleInfo64(hProcess, funcAddress, &moduleData))
			{
				Path filePath = moduleData.ImageName;

				outputStream << StringUtil::format(" Module[{0}]", filePath.getFilename());
			}
		}

		ls_free(buffer);

		return outputStream.str();
	}

	typedef bool(WINAPI *EnumProcessModulesType)(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);
	typedef DWORD(WINAPI *GetModuleBaseNameType)(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize);
	typedef DWORD(WINAPI *GetModuleFileNameExType)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize);
	typedef bool(WINAPI *GetModuleInformationType)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);

	static DynLib* gPSAPILib = nullptr;

	static EnumProcessModulesType gEnumProcessModules;
	static GetModuleBaseNameType gGetModuleBaseName;
	static GetModuleFileNameExType gGetModuleFileNameEx;
	static GetModuleInformationType gGetModuleInformation;

	/**	Dynamically load the PSAPI.dll and the required symbols, if not already loaded. */
	void win32_initPSAPI()
	{
		if (gPSAPILib != nullptr)
			return;

		gPSAPILib = ls_new<DynLib>("PSAPI.dll");
		gEnumProcessModules = (EnumProcessModulesType)gPSAPILib->getSymbol("EnumProcessModules");
		gGetModuleBaseName = (GetModuleBaseNameType)gPSAPILib->getSymbol("GetModuleFileNameExA");
		gGetModuleFileNameEx = (GetModuleFileNameExType)gPSAPILib->getSymbol("GetModuleBaseNameA");
		gGetModuleInformation = (GetModuleInformationType)gPSAPILib->getSymbol("GetModuleInformation");
	}

	/**	Unloads the PSAPI.dll if is loaded. */
	void win32_unloadPSAPI()
	{
		if (gPSAPILib == nullptr)
			return;

		gPSAPILib->unload();
		ls_delete(gPSAPILib);
		gPSAPILib = nullptr;
	}

	static bool gSymbolsLoaded = false;

	/**
	 * Loads symbols for all modules in the current process. Loaded symbols allow the stack walker to retrieve human
	 * readable method, file, module names and other information.
	 */
	void win32_loadSymbols()
	{
		if (gSymbolsLoaded)
			return;

		HANDLE hProcess = GetCurrentProcess();
		UINT32 options = SymGetOptions();

		options |= SYMOPT_LOAD_LINES;
		options |= SYMOPT_EXACT_SYMBOLS;
		options |= SYMOPT_UNDNAME;
		options |= SYMOPT_FAIL_CRITICAL_ERRORS;
		options |= SYMOPT_NO_PROMPTS;

		SymSetOptions(options);
		if(!SymInitialize(hProcess, nullptr, false))
		{
			LOGERR("SymInitialize failed. Error code: " + toString((UINT32)GetLastError()));
			return;
		}

		DWORD bufferSize;
		gEnumProcessModules(hProcess, nullptr, 0, &bufferSize);

		HMODULE* modules = (HMODULE*)ls_alloc(bufferSize);
		gEnumProcessModules(hProcess, modules, bufferSize, &bufferSize);

		UINT32 numModules = bufferSize / sizeof(HMODULE);
		for (UINT32 i = 0; i < numModules; i++)
		{
			MODULEINFO moduleInfo;

			char moduleName[LS_MAX_STACKTRACE_NAME_BYTES];
			char imageName[LS_MAX_STACKTRACE_NAME_BYTES];

			gGetModuleInformation(hProcess, modules[i], &moduleInfo, sizeof(moduleInfo));
			gGetModuleFileNameEx(hProcess, modules[i], imageName, LS_MAX_STACKTRACE_NAME_BYTES);
			gGetModuleBaseName(hProcess, modules[i], moduleName, LS_MAX_STACKTRACE_NAME_BYTES);

			char pdbSearchPath[LS_MAX_STACKTRACE_NAME_BYTES];
			char* fileName = nullptr;
			GetFullPathNameA(moduleName, LS_MAX_STACKTRACE_NAME_BYTES, pdbSearchPath, &fileName);
			*fileName = '\0';

			SymSetSearchPath(GetCurrentProcess(), pdbSearchPath);

			DWORD64 moduleAddress = SymLoadModule64(hProcess, modules[i], imageName, moduleName, (DWORD64)moduleInfo.lpBaseOfDll,
				(DWORD)moduleInfo.SizeOfImage);

			if (moduleAddress != 0)
			{
				IMAGEHLP_MODULE64 imageInfo;
				memset(&imageInfo, 0, sizeof(imageInfo));
				imageInfo.SizeOfStruct = sizeof(imageInfo);

				if(!SymGetModuleInfo64(GetCurrentProcess(), moduleAddress, &imageInfo))
				{
					LOGWRN("Failed retrieving module info for module: " + String(moduleName) + ". Error code: " + toString((UINT32)GetLastError()));
				}
				else
				{
					// Disabled because too much spam in the log, enable as needed
#if 0
					if (imageInfo.SymType == SymNone)
						LOGWRN("Failed loading symbols for module: " + String(moduleName));
#endif
				}
			}
			else
			{
				LOGWRN("Failed loading module " + String(moduleName) + ".Error code: " + toString((UINT32)GetLastError()) +
					". Search path: " + String(pdbSearchPath) + ". Image name: " + String(imageName));
			}
		}

		ls_free(modules);
		gSymbolsLoaded = true;
	}

	/**	Converts an exception record into a human readable error message. */
	String win32_getExceptionMessage(EXCEPTION_RECORD* record)
	{
		String exceptionAddress = toString((UINT64)record->ExceptionAddress, 0, ' ', std::ios::hex);

		String format;
		switch (record->ExceptionCode)
		{
		case EXCEPTION_ACCESS_VIOLATION:
			{
				DWORD_PTR violatedAddress = 0;
				if (record->NumberParameters == 2)
				{
					if (record->ExceptionInformation[0] == 0)
						format = "Unhandled exception at 0x{0}. Access violation reading 0x{1}.";
					else if (record->ExceptionInformation[0] == 8)
						format = "Unhandled exception at 0x{0}. Access violation DEP 0x{1}.";
					else
						format = "Unhandled exception at 0x{0}. Access violation writing 0x{1}.";

					violatedAddress = record->ExceptionInformation[1];
				}
				else
					format = "Unhandled exception at 0x{0}. Access violation.";

				String violatedAddressStr = toString((UINT64)violatedAddress, 0, ' ', std::ios::hex);
				return StringUtil::format(format, exceptionAddress, violatedAddressStr);
			}
		case EXCEPTION_IN_PAGE_ERROR:
		{
			DWORD_PTR violatedAddress = 0;
			DWORD_PTR code = 0;
			if (record->NumberParameters == 3)
			{
				if (record->ExceptionInformation[0] == 0)
					format = "Unhandled exception at 0x{0}. Page fault reading 0x{1} with code 0x{2}.";
				else if (record->ExceptionInformation[0] == 8)
					format = "Unhandled exception at 0x{0}. Page fault DEP 0x{1} with code 0x{2}.";
				else
					format = "Unhandled exception at 0x{0}. Page fault writing 0x{1} with code 0x{2}.";

				violatedAddress = record->ExceptionInformation[1];
				code = record->ExceptionInformation[3];
			}
			else
				format = "Unhandled exception at 0x{0}. Page fault.";

			String violatedAddressStr = toString((UINT64)violatedAddress, 0, ' ', std::ios::hex);
			String codeStr = toString((UINT64)code, 0, ' ', std::ios::hex);
			return StringUtil::format(format, exceptionAddress, violatedAddressStr, codeStr);
		}
		case STATUS_ARRAY_BOUNDS_EXCEEDED:
		{
			format = "Unhandled exception at 0x{0}. Attempting to access an out of range array element.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_DATATYPE_MISALIGNMENT:
		{
			format = "Unhandled exception at 0x{0}. Attempting to access missaligned data.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		{
			format = "Unhandled exception at 0x{0}. Floating point operand too small.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		{
			format = "Unhandled exception at 0x{0}. Floating point operation attempted to divide by zero.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_INVALID_OPERATION:
		{
			format = "Unhandled exception at 0x{0}. Floating point invalid operation.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_OVERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Floating point overflow.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_UNDERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Floating point underflow.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_STACK_CHECK:
		{
			format = "Unhandled exception at 0x{0}. Floating point stack overflow/underflow.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		{
			format = "Unhandled exception at 0x{0}. Attempting to execute an illegal instruction.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_PRIV_INSTRUCTION:
		{
			format = "Unhandled exception at 0x{0}. Attempting to execute a private instruction.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		{
			format = "Unhandled exception at 0x{0}. Integer operation attempted to divide by zero.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_INT_OVERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Integer operation result has overflown.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_STACK_OVERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Stack overflow.";
			return StringUtil::format(format, exceptionAddress);
		}
		default:
		{
			format = "Unhandled exception at 0x{0}. Code 0x{1}.";

			String exceptionCode = toString((UINT32)record->ExceptionCode, 0, ' ', std::ios::hex);
			return StringUtil::format(format, exceptionAddress, exceptionCode);
		}
		}
	}

	struct MiniDumpParams
	{
		Path filePath;
		EXCEPTION_POINTERS* exceptionData;
	};

	DWORD CALLBACK win32_writeMiniDumpWorker(void* data)
	{
		MiniDumpParams* params = (MiniDumpParams*)data;

		WString pathString = UTF8::toWide(params->filePath.toString());
		HANDLE hFile = CreateFileW(pathString.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
			nullptr);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;

			DumpExceptionInfo.ThreadId = GetCurrentThreadId();
			DumpExceptionInfo.ExceptionPointers = params->exceptionData;
			DumpExceptionInfo.ClientPointers = false;

			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal,
				&DumpExceptionInfo, nullptr, nullptr);
			CloseHandle(hFile);
		}

		return 0;
	}

	void win32_writeMiniDump(const Path& filePath, EXCEPTION_POINTERS* exceptionData)
	{
		MiniDumpParams param = { filePath, exceptionData };

		// Write minidump on a second thread in order to preserve the current thread's call stack
		DWORD threadId = 0;
		HANDLE hThread = CreateThread(nullptr, 0, &win32_writeMiniDumpWorker, &param, 0, &threadId);

		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}

	struct CrashHandler::Data
	{
		Mutex mutex;
	};

	void win32_popupErrorMessageBox(const WString& msg, const Path& folder)
	{
		WString simpleErrorMessage = msg
			+ L"\n\nFor more information check the crash report located at:\n "
			+ UTF8::toWide(folder.toString());
		MessageBoxW(nullptr, simpleErrorMessage.c_str(), L"Banshee fatal error!", MB_OK);

	}
	void CrashHandler::reportCrash(const String& type, const String& description, const String& function, 
		const String& file, UINT32 line) const
	{
		// Win32 debug methods are not thread safe
		Lock lock(m->mutex);

		logErrorAndStackTrace(type, description, function, file, line);
		saveCrashLog();

		win32_writeMiniDump(getCrashFolder() + String(sMiniDumpName), nullptr);
		win32_popupErrorMessageBox(toWString(sFatalErrorMsg), getCrashFolder());

		DebugBreak();

		// Note: Potentially also log Windows Error Report and/or send crash data to server
	}

	int CrashHandler::reportCrash(void* exceptionDataPtr) const
	{
		EXCEPTION_POINTERS* exceptionData = (EXCEPTION_POINTERS*)exceptionDataPtr;

		// Win32 debug methods are not thread safe
		Lock lock(m->mutex);

		win32_initPSAPI();
		win32_loadSymbols();

		logErrorAndStackTrace(win32_getExceptionMessage(exceptionData->ExceptionRecord),
			win32_getStackTrace(*exceptionData->ContextRecord, 0));
		saveCrashLog();

		win32_writeMiniDump(getCrashFolder() + String(sMiniDumpName), exceptionData);
		win32_popupErrorMessageBox(toWString(sFatalErrorMsg), getCrashFolder());

		DebugBreak();

		// Note: Potentially also log Windows Error Report and/or send crash data to server

		return EXCEPTION_EXECUTE_HANDLER;
	}

	String CrashHandler::getCrashTimestamp()
	{
		SYSTEMTIME systemTime;
		GetLocalTime(&systemTime);

		String timeStamp = "{0}{1}{2}_{3}{4}";
		String strYear = toString(systemTime.wYear, 4, '0');
		String strMonth = toString(systemTime.wMonth, 2, '0');
		String strDay = toString(systemTime.wDay, 2, '0');
		String strHour = toString(systemTime.wHour, 2, '0');
		String strMinute = toString(systemTime.wMinute, 2, '0');
		return StringUtil::format(timeStamp, strYear, strMonth, strDay, strHour, strMinute);
	}

	String CrashHandler::getStackTrace()
	{
		CONTEXT context;
		RtlCaptureContext(&context);

		win32_initPSAPI();
		win32_loadSymbols();
		return win32_getStackTrace(context, 2);
	}
}

#endif // PLATFORM_WINDOWS