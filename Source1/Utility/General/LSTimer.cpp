#include "General/LSTimer.h"
#include "General/LSBitwise.h"

#include <chrono>

using namespace std::chrono;

namespace ls
{
	Timer::Timer()
	{
		reset();
	}

	void Timer::reset()
	{
		mStartTime = mHRClock.now();
	}

	UINT64 Timer::getMilliseconds() const
	{
		auto newTime = mHRClock.now();
		duration<double> dur = newTime - mStartTime;

		return duration_cast<milliseconds>(dur).count();
	}

	UINT64 Timer::getMicroseconds() const
	{
		auto newTime = mHRClock.now();
		duration<double> dur = newTime - mStartTime;

		return duration_cast<microseconds>(dur).count();
	}

	UINT64 Timer::getStartMs() const
	{
		nanoseconds startTimeNs = mStartTime.time_since_epoch();

		return duration_cast<milliseconds>(startTimeNs).count();
	}
}
