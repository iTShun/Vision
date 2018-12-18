#include "Math/LSVector4.h"
#include "Math/LSMath.h"

namespace ls
{
	const Vector4 Vector4::ZERO{LS_ZERO()};

	bool Vector4::isNaN() const
	{
		return Math::isNaN(x) || Math::isNaN(y) || Math::isNaN(z) || Math::isNaN(w);
	}
}
