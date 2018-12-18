#pragma once

#include "Testing/LSTestSuite.h"

namespace ls
{
	class UtilityTestSuite : public TestSuite
	{
	public:
		UtilityTestSuite();
		void startUp() override;
		void shutDown() override;

	private:
		void testBitfield();
		void testOctree();
		void testSmallVector();
		void testDynArray();
		void testComplex();
	};
}
