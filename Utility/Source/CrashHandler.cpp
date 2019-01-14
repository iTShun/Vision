#include "CrashHandler.h"
#include "FileSystem.h"
#include "Debug.h"

#if PLATFORM_WINDOWS
#   include <windows.h>
#   include <psapi.h>

// Disable warning in VS2015 that's not under my control
#   pragma warning(disable : 4091)
#   include <DbgHelp.h>
#   include "Unicode.h"
#   pragma warning(default : 4091)

#   if COMPILER_MSVC
#       pragma comment(lib, "DbgHelp.lib")
#   endif

static const char* sMiniDumpName = "minidump.dmp";
#else
#   include <cxxabi.h>
#   include <execinfo.h>
#   include <dlfcn.h>
#   include <csignal>
#endif

namespace Utility
{
	const String CrashHandler::sCrashReportFolder = "CrashReports";
	const String CrashHandler::sCrashLogName = u8"log.html";
	const String CrashHandler::sFatalErrorMsg =
		"A fatal error occurred and the program has to terminate!";

#   if PLATFORM_WINDOWS
    struct Data
    {
        Mutex mutex;
    };
    
    struct MiniDumpParams
    {
        Path filePath;
        EXCEPTION_POINTERS* exceptionData;
    };
    
    Data* gData = nullptr;
    
    UINT32 win32_getRawStackTrace(CONTEXT context, UINT64 stackTrace[MAX_STACKTRACE_DEPTH]);
    String win32_getStackTrace(CONTEXT context, UINT32 skip = 0);
    String win32_getExceptionMessage(EXCEPTION_RECORD* record);
    DWORD CALLBACK win32_writeMiniDumpWorker(void* data);
    void win32_writeMiniDump(const Path& filePath, EXCEPTION_POINTERS* exceptionData);
    void win32_popupErrorMessageBox(const WString& msg, const Path& folder);
#   else
    INT32 SIGNALS[] = { SIGFPE, SIGILL, SIGSEGV, SIGTERM };
    struct sigaction gSavedSignals[4];
    
    void signalHandler(int signal, siginfo_t* info, void* context);
#   endif
    
    CrashHandler& gCrashHandler()
    {
        return CrashHandler::instance();
    }
    
    CrashHandler::CrashHandler()
    {
#   if PLATFORM_WINDOWS
        gData = New<Data>();
#   else
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_sigaction = &signalHandler;
        action.sa_flags = SA_SIGINFO;
        
        INT32 i = 0;
        for(auto& entry : SIGNALS)
        {
            memset(&gSavedSignals[i], 0, sizeof(struct sigaction));
            sigaction(entry, &action, &gSavedSignals[i]);
            
            i++;
        }
#   endif
    }
    
    CrashHandler::~CrashHandler()
    {
#   if PLATFORM_WINDOWS
        Delete(gData);
#   else
        
#   endif
    }
    
    void CrashHandler::reportCrash(const String& type,
                                   const String& description,
                                   const String& function,
                                   const String& file,
                                   UINT32 line) const
    {
#   if PLATFORM_WINDOWS
        // Win32 debug methods are not thread safe
        Lock lock(m->mutex);
        
        logErrorAndStackTrace(type, description, function, file, line);
        saveCrashLog();
        
        win32_writeMiniDump(getCrashFolder() + String(sMiniDumpName), nullptr);
        win32_popupErrorMessageBox(toWString(sFatalErrorMsg), getCrashFolder());
        
        DebugBreak();
        
        // Note: Potentially also log Windows Error Report and/or send crash data to server
#   else
        logErrorAndStackTrace(type, description, function, file, line);
        saveCrashLog();
        
        // Allow the debugger a chance to attach
        std::raise(SIGINT);
#   endif
    }
    
#if PLATFORM_WINDOWS
    int CrashHandler::reportCrash(void* exceptionData) const
    {
        EXCEPTION_POINTERS* exceptionData = (EXCEPTION_POINTERS*)exceptionDataPtr;
        
        // Win32 debug methods are not thread safe
        Lock lock(m->mutex);
        
        logErrorAndStackTrace(win32_getExceptionMessage(exceptionData->ExceptionRecord),
                              win32_getStackTrace(*exceptionData->ContextRecord, 0));
        saveCrashLog();
        
        win32_writeMiniDump(getCrashFolder() + String(sMiniDumpName), exceptionData);
        win32_popupErrorMessageBox(toWString(sFatalErrorMsg), getCrashFolder());
        
        DebugBreak();
        
        // Note: Potentially also log Windows Error Report and/or send crash data to server
        
        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif
    
    String CrashHandler::getStackTrace()
    {
#   if PLATFORM_WINDOWS
        CONTEXT context;
        memset(&context, 0, sizeof(context));
        context.ContextFlags = (CONTEXT_FULL);
        RtlCaptureContext(&context);
        
        return win32_getStackTrace(context, 2);
#   else
        StringStream stackTrace;
        void* trace[MAX_STACKTRACE_DEPTH];
        
        int traceSize = backtrace(trace, MAX_STACKTRACE_DEPTH);
        char** messages = backtrace_symbols(trace, traceSize);
        
        for (int i = 0; i < traceSize && messages != nullptr; ++i)
        {
#       if PLATFORM_OSX
            stackTrace << std::to_string(i) << ") " << messages[i];
            
            // Try parsing a human readable name
            Dl_info info;
            if (dladdr(trace[i], &info) && info.dli_sname)
            {
                stackTrace << ": ";
                
                if (info.dli_sname[0] == '_')
                {
                    int status = -1;
                    char* demangledName = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
                    
                    if(status == 0)
                        stackTrace << demangledName;
                    else
                        stackTrace << info.dli_sname;
                    
                    free(demangledName);
                }
                else
                    stackTrace << info.dli_sname;
                
                // Try to find the line number
                for (char *p = messages[i]; *p; ++p)
                {
                    if (*p == '+')
                    {
                        stackTrace << " " << p;
                        break;
                    }
                }
            }
            else
                stackTrace << String(messages[i]);
#       elif PLATFORM_LINUX
            // Try to find the characters surrounding the mangled name: '(' and '+'
            char* mangedName = nullptr;
            char* offsetBegin = nullptr;
            char* offsetEnd = nullptr;
            for (char *p = messages[i]; *p; ++p)
            {
                if (*p == '(')
                    mangedName = p;
                else if (*p == '+')
                    offsetBegin = p;
                else if (*p == ')')
                {
                    offsetEnd = p;
                    break;
                }
            }
            
            bool lineContainsMangledSymbol = mangedName != nullptr && offsetBegin != nullptr && offsetEnd != nullptr &&
            mangedName < offsetBegin;
            
            stackTrace << toString(i) << ") ";
            
            if (lineContainsMangledSymbol)
            {
                *mangedName++ = '\0';
                *offsetBegin++ = '\0';
                *offsetEnd++ = '\0';
                
                int status;
                char *real_name = abi::__cxa_demangle(mangedName, 0, 0, &status);
                char *output_name = status == 0 /* Demangling successful */? real_name : mangedName;
                stackTrace << String(messages[i])
                << ": " << output_name
                << "+" << offsetBegin << offsetEnd;
                
                free(real_name);
            }
            else
                stackTrace << String(messages[i]);
#       endif
            
            if (i < traceSize - 1)
                stackTrace << "\n";
        }
        
        free(messages);
        
        return stackTrace.str();
#   endif
    }

	const Path& CrashHandler::getCrashFolder()
	{
		static const Path path = FileSystem::getWorkingDirectoryPath() + sCrashReportFolder + 
			getCrashTimestamp();
		
		static bool first = true;
		if (first) 
		{
            FileSystem::createDir(path);
			first = false;
		}

		return path;
	}

	void CrashHandler::logErrorAndStackTrace(const String& errorMsg, const String& stackTrace) const
	{
		StringStream errorMessage;
		errorMessage << sFatalErrorMsg << std::endl;
		errorMessage << errorMsg;
		errorMessage << "\n\nStack trace: \n";
		errorMessage << stackTrace;

        gDebug().logError(errorMessage.str());
	}

	void CrashHandler::logErrorAndStackTrace(const String& type, const String& description, const String& function,
		const String& file, UINT32 line) const
	{
		StringStream errorMessage;
		errorMessage << "  - Error: " << type << std::endl;
		errorMessage << "  - Description: " << description << std::endl;
		errorMessage << "  - In function: " << function << std::endl;
		errorMessage << "  - In file: " << file << ":" << line;
		logErrorAndStackTrace(errorMessage.str(), getStackTrace());
	}

	void CrashHandler::saveCrashLog() const
	{
        gDebug().saveLog(getCrashFolder() + sCrashLogName);
	}
    
    String CrashHandler::getCrashTimestamp()
    {
#   if PLATFORM_WINDOWS
        SYSTEMTIME systemTime;
        GetLocalTime(&systemTime);
        
        String timeStamp = "{0}{1}{2}_{3}{4}";
        String strYear = toString(systemTime.wYear, 4, '0');
        String strMonth = toString(systemTime.wMonth, 2, '0');
        String strDay = toString(systemTime.wDay, 2, '0');
        String strHour = toString(systemTime.wHour, 2, '0');
        String strMinute = toString(systemTime.wMinute, 2, '0');
        return StringUtil::format(timeStamp, strYear, strMonth, strDay, strHour, strMinute);
#   else
        std::time_t t = time(0);
        struct tm *now = localtime(&t);
        
        String timeStamp = "{0}{1}{2}_{3}{4}";
        String strYear = toString(now->tm_year, 4, '0');
        String strMonth = toString(now->tm_mon, 2, '0');
        String strDay = toString(now->tm_mday, 2, '0');
        String strHour = toString(now->tm_hour, 2, '0');
        String strMinute = toString(now->tm_min, 2, '0');
        return StringUtil::format(timeStamp, strYear, strMonth, strDay, strHour, strMinute);
#   endif
    }
    
#   if PLATFORM_WINDOWS
    /**
     * Returns the raw stack trace using the provided context. Raw stack trace contains only function addresses.
     *
     * @param[in]    context        Processor context from which to start the stack trace.
     * @param[in]    stackTrace    Output parameter that will contain the function addresses. First address is the deepest
     *                             called function and following address is its caller and so on.
     * @return                    Number of functions in the call stack.
     */
    UINT32 win32_getRawStackTrace(CONTEXT context, UINT64 stackTrace[MAX_STACKTRACE_DEPTH])
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
            
            if (numEntries < MAX_STACKTRACE_DEPTH)
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
     * @param[in]    context        Processor context from which to start the stack trace.
     * @param[in]    skip        Number of bottom-most call stack entries to skip.
     * @return                    String containing the call stack with each function on its own line.
     */
    String win32_getStackTrace(CONTEXT context, UINT32 skip = 0)
    {
        UINT64 rawStackTrace[MAX_STACKTRACE_DEPTH];
        UINT32 numEntries = win32_getRawStackTrace(context, rawStackTrace);
        
        numEntries = std::min((UINT32)MAX_STACKTRACE_DEPTH, numEntries);
        
        UINT32 bufferSize = sizeof(PIMAGEHLP_SYMBOL64) + MAX_STACKTRACE_NAME_BYTES;
        UINT8* buffer = (UINT8*)Alloc(bufferSize);
        
        PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)buffer;
        symbol->SizeOfStruct = bufferSize;
        symbol->MaxNameLength = MAX_STACKTRACE_NAME_BYTES;
        
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
            IMAGEHLP_LINE64    lineData;
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
        
        Free(buffer);
        
        return outputStream.str();
    }
    
    /**    Converts an exception record into a human readable error message. */
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
    
    void win32_popupErrorMessageBox(const WString& msg, const Path& folder)
    {
        WString simpleErrorMessage = msg
        + L"\n\nFor more information check the crash report located at:\n "
        + UTF8::toWide(folder.toString());
        MessageBoxW(nullptr, simpleErrorMessage.c_str(), L"Banshee fatal error!", MB_OK);
    }
#   else
    void signalHandler(int signal, siginfo_t* info, void* context)
    {
        // Restore old signal handlers
        INT32 i = 0;
        for(auto& entry : SIGNALS)
        {
            sigaction(entry, &gSavedSignals[i], nullptr);
            i++;
        }
        
        const char* signalNameSz = strsignal(signal);
        
        String signalName;
        if(signalNameSz)
            signalName = signalNameSz;
        else
            signalName = "Unknown signal #" + toString(signal);
        
        // Note: Not safe to grab a stack-trace here (nor do memory allocations), but we might as well try since we're
        // crashing anyway
        CrashHandler::instance().reportCrash(signalName, "Received fatal signal", "", "");
        
        kill(getpid(), signal);
        exit(signal);
    }
#   endif
}
