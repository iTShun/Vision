#include "Testing/LSTestSuite.h"
#include "Testing/LSTestOutput.h"

namespace ls
{
	TestSuite::TestEntry::TestEntry(Func test, const String& name)
		:test(test), name(name)
	{ }

	void TestSuite::run(TestOutput& output)
	{
		mOutput = &output;

		startUp();

		for (auto& testEntry : mTests)
		{
			mActiveTestName = testEntry.name;
			
			(this->*(testEntry.test))();
		}

		for (auto& suite : mSuites)
		{
			suite->run(output);
		}

		shutDown();
	}

	void TestSuite::add(const SPtr<TestSuite>& suite)
	{
		mSuites.push_back(suite);
	}

	void TestSuite::addTest(Func test, const String& name)
	{
		mTests.push_back(TestEntry(test, name));
	}

	void TestSuite::assertment(bool success, const String& desc, const String& file, long line)
	{
		if (!success)
			mOutput->outputFail(desc, mActiveTestName, file, line);
	}
}
