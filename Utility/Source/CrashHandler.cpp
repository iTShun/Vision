#include "CrashHandler.h"
#include "FileSystem.h"
#include "Debug.h"

namespace Utility
{
	const String CrashHandler::sCrashReportFolder = "CrashReports";
	const String CrashHandler::sCrashLogName = u8"log.html";
	const String CrashHandler::sFatalErrorMsg =
		"A fatal error occurred and the program has to terminate!";

    CrashHandler& gCrashHandler()
    {
        return CrashHandler::instance();
    }
    
    CrashHandler::CrashHandler()
    {
        
    }
    
    CrashHandler::~CrashHandler()
    {
        
    }
    
    void CrashHandler::reportCrash(const String& type,
                                   const String& description,
                                   const String& function,
                                   const String& file,
                                   UINT32 line) const
    {
        
    }
    
#if PLATFORM_WINDOWS
    int CrashHandler::reportCrash(void* exceptionData) const
    {
        return 0;
    }
#endif
    
    String CrashHandler::getStackTrace()
    {
        return String("");
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
        return String("");
    }
}
