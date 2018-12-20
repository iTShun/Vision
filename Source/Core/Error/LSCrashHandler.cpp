#include "LSCrashHandler.h"
#include "Logger/LSLogger.h"
#include "FileSystem/LSFileSystem.h"
#include "FileSystem/LSPath.h"

namespace ls
{
	const String CrashHandler::sCrashReportFolder = "CrashReports";
	const String CrashHandler::sCrashLogName = u8"log.html";
	const String CrashHandler::sFatalErrorMsg =
		"A fatal error occurred and the program has to terminate!";

	CrashHandler& gCrashHandler()
	{
		return CrashHandler::instance();
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

		gLogger().logError(errorMessage.str());
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
		gLogger().saveLog(getCrashFolder() + sCrashLogName);
	}
}
