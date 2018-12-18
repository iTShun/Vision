#include "Prerequisites/LSPrerequisitesUtil.h"
#include "Math/LSRadian.h"
#include "Math/LSMath.h"

namespace ls
{
	Radian::Radian (const Degree& d) : mRad(d.valueRadians())
	{ }

	Radian Radian::wrap()
	{
		mRad = fmod(mRad, Math::TWO_PI);

		if (mRad < 0)
			mRad += Math::TWO_PI;

		return *this;
	}

	Radian& Radian::operator= (const Degree& d)
	{
		mRad = d.valueRadians(); 
		return *this;
	}

	Radian Radian::operator+ (const Degree& d) const
	{
		return Radian (mRad + d.valueRadians());
	}

	Radian& Radian::operator+= (const Degree& d)
	{
		mRad += d.valueRadians();
		return *this;
	}

	Radian Radian::operator- (const Degree& d) const
	{
		return Radian (mRad - d.valueRadians());
	}

	Radian& Radian::operator-= (const Degree& d)
	{
		mRad -= d.valueRadians();
		return *this;
	}

	float Radian::valueDegrees() const
	{
		return mRad * Math::RAD2DEG;
	}
}
