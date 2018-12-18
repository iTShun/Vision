#include "Testing/LSTestOutput.h"
#include "Error/LSException.h"

namespace ls
{
	void ExceptionTestOutput::outputFail(const String& desc, const String& function, const String& file, long line)
	{
		LS_EXCEPT(UnitTestException, desc);
	}
}
