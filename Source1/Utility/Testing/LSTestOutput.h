#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"

namespace ls
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Abstract interface used for outputting unit test results. */
	class LS_UTILITY_EXPORT TestOutput
	{
	public:
		virtual ~TestOutput() {}

		/**
		 * Triggered when a unit test fails.
		 *
		 * @param[in]	desc		Reason why the unit test failed.
		 * @param[in]	function	Name of the function the test failed in.
		 * @param[in]	file		File the unit test failed in.
		 * @param[in]	line		Line of code the unit test failed on.
		 */
		virtual void outputFail(const String& desc, const String& function, const String& file, long line) = 0;
	};

	/** Outputs unit test results so that failures are reported as exceptions. Success is not reported. */
	class LS_UTILITY_EXPORT ExceptionTestOutput : public TestOutput
	{
	public:
		/** @copydoc TestOutput::outputFail */
		void outputFail(const String& desc, const String& function, const String& file, long line) final override;
	};

	/** @} */
}