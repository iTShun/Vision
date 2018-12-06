#include "Testing/LSConsoleTestOutput.h"
#include "Private/UnitTests/LSUtilityTestSuite.h"

using namespace ls;

int main()
{
	SPtr<TestSuite> tests = UtilityTestSuite::create<UtilityTestSuite>();

	ConsoleTestOutput testOutput;
	tests->run(testOutput);

	return 0;
}
