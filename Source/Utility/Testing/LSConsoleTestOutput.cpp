#include "Testing/LSConsoleTestOutput.h"

#include <iostream>

namespace ls
{
	void ConsoleTestOutput::outputFail(const String& desc,
	                                   const String& function,
	                                   const String& file,
	                                   long line)
	{
		std::cout << file << ":" << line << ": failure: " << desc << std::endl;
	}
}
