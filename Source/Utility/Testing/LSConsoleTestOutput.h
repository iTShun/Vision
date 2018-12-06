#pragma once

#include "Testing/LSTestOutput.h"
#include "Prerequisites/LSPrerequisitesUtil.h"

namespace ls
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Outputs unit test failures to stdout. */
	class LS_UTILITY_EXPORT ConsoleTestOutput : public TestOutput
	{
	public:
		/** @copydoc TestOutput::outputFail */
		void outputFail(const String& desc,
		                const String& function,
		                const String& file,
		                long line) final override;
	};

	/** @} */
}
